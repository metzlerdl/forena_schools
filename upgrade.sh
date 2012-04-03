#!/bin/sh
#-------------------------------------
# Database Connection
#--------------------------------------------
PGDATABASE="psd"
PGUSER="webdev"
PGPASS="f1nt0ok"
PSQL_CMD="psql -U $PGUSER $PGDATABASE"
#---------------------------------------------------------------------------------------
# Uncomment the following command to automatically perform a db backup before upgrades. 
# WARNING:  This backup command doesn't check for successful completion, and will happily 
#  perform an upgrade even if the backup fails.  It's more meant for an extra safegaurd.  
#  There's no substitute for good backups!
#--------------------------------------------------------------------------------------
#PG_DUMP_CMD="pg_dump $PGDATABASE -U $PGUSER" 
#------ Environment variables for auto copy install
# Specify the directory that you want to copy the web files to upon completion of the install for
# the purposes of deploying the source code. Do not include trailing slashes. 
WEB_DIR="/htdocs/psdims"
if [ -f ~/.pedagoggle_env ] ;then 
  . ~/.pedagoggle_env
fi 
#----------------------------------------------------------------- 
# DO NOT MODIFY BELOW THIS LINE
#-----------------------------------------------------------------
PEDAGOGGLE_HOME=`pwd`
PATH="$PEDAGOGGLE_HOME/scripts:$PATH"
export PATH PEDAGOGGLE_HOME PGPASS PSQL_CMD PG_DUMP_CMD
# Perform database upgrade
. scripts/upgrade_db.sh
# Perorm file copies assumes ability to overwrite files in pedagoggle directory. 
cd $PEDAGOGGLE_HOME
if [ -n "$WEB_DIR" ]; then 
  rm -rf $WEB_DIR/data
  rm -rf $WEB_DIR/flex
  echo "Copying files to $WEB_DIR"
  cp -r web/*  $WEB_DIR/ 
fi 