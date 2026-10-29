"""Offline serialization and cross-DLL SKSE RTTI checks; never load skee64/game."""
import argparse
import importlib.util
import json
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    output = args.output.resolve()
    if output.exists() or output.is_relative_to(ROOT):
        raise ValueError('Use a fresh output directory outside Git')
    output.mkdir(parents=True)
    sys.pycache_prefix = str(output / 'pycache')
    spec = importlib.util.spec_from_file_location('lifetime_build', ROOT / 'tools/test-string-table-lifetime.py')
    helper = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(helper)
    env = helper.msvc_environment(helper.DEFAULT_VCVARS, output)
    compiler = shutil.which('cl.exe', path=env['PATH'])
    fixtures = ROOT / 'tests/native-contracts'
    headers = output / 'headers'
    headers.mkdir()
    pins = []
    for name in ('SKEESerialization.h', 'SKEEDelayFunctors.h', 'StringTable.h'):
        target = headers / name
        shutil.copyfile(ROOT / 'skee64' / name, target)
        pins.append(helper.pin(target))
    common = [compiler, '/nologo', '/std:c++20', '/EHsc', '/MD', '/Od',
              '/I' + str(headers),
              '/I' + str(fixtures / 'stubs'),
              '/I' + str(ROOT / 'tests/string-table-lifetime/stubs')]
    builds = [
        ('serialization', [*common, str(fixtures / 'serialization.cpp'), '/Fe:serialization.exe']),
        ('provider', [*common, '/LD', str(fixtures / 'rtti-provider.cpp'), '/Fe:rtti-provider.dll']),
        ('consumer', [*common, str(fixtures / 'rtti-consumer.cpp'), '/Fe:rtti-consumer.exe']),
    ]
    results = []
    for name, command in builds:
        result = subprocess.run(command, cwd=output, env=env, capture_output=True, text=True)
        (output / (name + '-build.log')).write_text(result.stdout + result.stderr, encoding='utf-8')
        if result.returncode:
            raise RuntimeError(name + ' build failed: inspect build log')
    for name, command in [('serialization', [str(output / 'serialization.exe')]),
                          ('rtti', [str(output / 'rtti-consumer.exe'), str(output / 'rtti-provider.dll')])]:
        result = subprocess.run(command, cwd=output, env=env, capture_output=True, text=True, timeout=30)
        log = output / (name + '.log')
        log.write_text(result.stdout + result.stderr, encoding='utf-8')
        results.append({'name': name, 'returnCode': result.returncode, 'passed': result.returncode == 0,
                        'log': helper.pin(log)})
    report = {'gameStarted': False, 'pluginLoaded': False,
              'method': 'actual production helper headers; narrow engine/serialization stubs; separate-DLL legacy global RTTI consumer',
              'sourcePins': pins, 'results': results, 'passed': all(row['passed'] for row in results)}
    (output / 'report.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
    print(json.dumps(report, indent=2))
    return 0 if report['passed'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
