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
mode=state_testing
bldg_code_field=schoolcode
sis_id_field=districtstudentcode
school_year=2011
grade_level=4
date_taken=2011-05-10
grade_level=3
3rd.2011.csv>imp_test_scores
etl_merge_student_from_test()
etl_merge_test_scores()
grade_level=4
4th.2011.csv>imp_test_scores
etl_merge_student_from_test()
etl_merge_test_scores()
grade_level=5
5th.2011.csv>imp_test_scores
etl_merge_student_from_test()
etl_merge_test_scores()
grade_level=6
6th.2011.csv>imp_test_scores
etl_merge_student_from_test()
etl_merge_test_scores()
grade_level=7
7th.2011.csv>imp_test_scores
etl_merge_student_from_test()
etl_merge_test_scores()
grade_level=8
8th.2011.csv>imp_test_scores
etl_merge_student_from_test()
etl_merge_test_scores()
test_code=MSP
etl_calc_score_stats(:school_year, :test_code)
test_code=MSPB
etl_calc_score_stats(:school_year, :test_code)
EOF