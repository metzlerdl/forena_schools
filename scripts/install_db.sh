#!/bin/sh
cd db_api
UPGRADE_DIR=.
# ------------------
# Checking for psql 
#------------------
psql_check=`psql --version`
if [ -z psql_check ]; then
  echo "No psql utility found in current path" 
  exit 1
fi 
if [ -z $LOG_DIR ]; then
  LOG_DIR=../logs
fi

#--------------------
#Extract The schema revision from the database
echo "Installing Database"
$PSQL_CMD -f pedagoggle.sql
$PSQL_CMD -f tables/insert_grade_levels.sql
$PSQL_CMD -f tables/insert_buildings.sql


