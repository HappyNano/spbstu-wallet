#!/bin/bash

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
    if [[ ! -z "$(git show :"$filename" | tail -c 1)" ]] then
        echo "$filename: No Newline at end of file!"
        errors=$(($errors + 1))
    fi
done

if [[ $errors != 0 ]]
then
    printf "$errors files need to be with EOF empty line\n"
    exit 1
fi
