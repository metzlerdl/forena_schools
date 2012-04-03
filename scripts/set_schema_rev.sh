#!/bin/sh
if [ -z "$1" ]; then
  echo "Usage $0 [Database] [Schema rev number]"
  exit 1 
fi 
echo "$PSQL_CMD"
$PSQL_CMD <<EOF
select f_set('schema_rev',CAST ('$1' AS VARCHAR)); 
\q
EOF