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
export PGPASS 
export PSQL_CMD
#--------------------
#Extract The schema revision from the database
SCHEMA_REV=`$PSQL_CMD -qAt <<EOF
SELECT f_get('schema_rev');
\q
EOF
`
if [ -z "$SCHEMA_REV" ]; then
  SCHEMA_REV="0.00"
fi 
echo "Current pedagoggle scheme version number is $SCHEMA_REV"
REVS=(${SCHEMA_REV//./ });
MAJOR=${REVS[0]}
MINOR=${REVS[1]}
# If we've defined a backup command run a backup before dumping the db. 
if [ -n "$PG_DUMP_CMD" ]; then 
  echo "Backup up database prior to executing upgrade, please wait"
  $PG_DUMP_CMD >backup_${MAJOR}_${MINOR}.sql
fi 
#Now we know the major and minor rev in the database So lets look in the upgrade directory for
#.sql files that need to be upgradeed. 
for file in $UPGRADE_DIR/upgrade*.sql
do
  file_name=`basename $file`
  file_parts=(${file_name//./ })
  ext=${file_parts[1]}
  base=${file_parts[0]}
  script_name=(${base//_/ })
  new_major=${script_name[1]}
  new_minor=${script_name[2]}
  if [ -n "$new_major" ] && [ -n "$new_minor" ]; then
	    UPGRADE_FLAG="N"
	    UPGRADE_REV="$new_major.$new_minor"
	    # Checking for Major version upgrade
	    if [ "$new_major" -gt "$MAJOR" ]; then 
	      echo "Major version upgrade detected $UPGRADE_REV"
	      UPGRADE_FLAG="Y"
	    fi 
	    # Checking for Minor version upgrade. 
	    if [ "$new_major" -eq "$MAJOR" ] && [ "$new_minor" -gt "$MINOR" ]; then
	        echo "Mindor version upgrade detected $UPGRADE_REV" 
	        UPGRADE_FLAG="Y"
	    fi
	    #Should we run this upgrade? 
	    if [ $UPGRADE_FLAG == "Y" ]; then 
	      #peform the upgrade
	      echo "Upgrading to $new_major.$new_minor"
	      $PSQL_CMD -f $file >$LOG_DIR/upgrade_${new_major}_${new_minor}.log
	      echo "Reseting schema var"
	      set_schema_rev.sh $UPGRADE_REV
	      LAST_REV=$UPGRADE_REV
	      echo "Upgrade to $UPGRADE_REV Complete"
	    fi   
  fi
done

if [ -n "$LAST_REV" ]; then 
  echo "Upgraded to $LAST_REV"
  echo "Database Upgraded to $LAST_REV">>$LOG_DIR/upgrade.log
else
  echo "No upgrades detected." 
fi 
