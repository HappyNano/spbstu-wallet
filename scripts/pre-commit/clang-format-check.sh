#!/bin/bash

if git rev-parse --verify HEAD >/dev/null 2>&1
then
    against=HEAD
else
    against=$(git hash-object -t tree /dev/null)
fi

exec 1>&2

errors=0

for filename in $(git diff --cached --name-only $against)
do
    if [[ ! -f $filename ]]
    then
        echo "[clang-format-checks] File $filename deleted"
    elif [[ $filename =~ .*\.(h|hpp|c|cpp)$ ]]
    then
        diffs=$(diff -u <(git show :"$filename") <(git show :"$filename" | clang-format))

        if [[ ! -z $diffs ]]
        then
            printf "[clang-format-checks] Clang-format error with file $filename:\n$diffs\n"
            errors=$(($errors + 1))
        fi
    fi
done

if [[ $errors != 0 ]]
then
    printf "$errors files code format need to be fixed\n"
    exit 1
fi
