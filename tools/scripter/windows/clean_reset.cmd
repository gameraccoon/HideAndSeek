@echo off

git clean -ffdx -e .scripter_config.local.json
git reset --hard HEAD
