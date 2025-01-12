#!/bin/sh
#

for i in $(ls *.dot)
do
    dot -Tsvg "${i}" > "$(basename ${i} '.dot').svg"
done

exit 0
