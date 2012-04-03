#/bin/bash
# The following is a sample import shell script that can be used to import 
# data into pedagoggle. Copy this script and modify the code to import the files from the locations that you want. 
# It's probably best to fully qualify the path name. 
#
#
# Syntax Summary: 
#    variable=value      --- Sets the default value for any field.  This can be used to set important information such as school codes or 
#    filename>tablename  --- Imports a csv file into an import table
#    function()          --- Calls a database function to perform the actual import. 
# 
#
# Uncomment the following line to change directory
# cd {pathtopdedagggle}/scripts
php pedacmd.php <<EOF
import_directory=/Users/davidmetzler/pedagoggle_import
school_year=2012
#ims_students.txt>imp_students
#etl_merge_students()
role=teacher
ims_teachers.txt>imp_staff
etl_merge_staff()
ims_courses.txt>imp_courses
ims_schedules.txt>imp_course_schedules
etl_merge_courses()
etl_merge_course_schedules()
EOF