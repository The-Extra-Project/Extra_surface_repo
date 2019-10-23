#!/bin/bash

function eval_exe {
    echo "---------------------------"
    echo "exec : $EXEC"
    echo ""
    eval $EXEC
    local status=$?
    if [ $status -ne 0 ]; then
        echo "/!\ /!\ error /!\ /!\ " >&2
        echo "error when executing :" >&2
        echo "$EXEC" >&2
        echo "/!\ /!\ error /!\ /!\ " >&2
        kill $$
        exit 1;
    else
	echo "exec : $EXEC"
        echo "OK : return 0"
    fi
    return $status
}


VNUM="1.0"
current_dir_name=${PWD##*/} 
if [ $# -eq 0 ]
then
    NAME="bk"
else
    NAME=$1
fi

NAME="${current_dir_name}_${VNUM}_$(date +%Y-%m-%d)_$NAME.tar.gz"
EXEC="tar -czvf ~/bcp/$NAME --exclude='build*' --exclude='.git*' --exclude='*.tar.gz' --exclude='metastore_db'  ./"
eval_exe
EXEC="scp -r ~/bcp/$NAME laurent@lcaraffa.ddns.net:/home/laurent/scp/"
eval_exe
