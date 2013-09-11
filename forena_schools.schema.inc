<?php
/**
 * Implementation of hook_schema.
 */
function forena_schools_schema() {

  $schema['i_buildings'] = array(
    'description' => 'buildings',
    'fields' => array(
      'bldg_id' => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'size' => 'big',
        'not null' => TRUE,
      ),

      'name' => array(
        'type' => 'varchar',
        'length' => 75,
        'not null' => TRUE,
      ),

      'abbrev' => array(
        'type' => 'varchar',
        'length' => 25,
      ),

      'code' => array(
        'type' => 'varchar',
        'length' => 25,
      ),

      'sis_code' => array(
        'type' => 'varchar',
        'length' => 25,
      ),

      'min_grade' => array(
        'type' => 'int',
        'size' => 'small',
        'unsigned' => TRUE,
      ),

      'max_grade' => array(
        'type' => 'int',
        'size' => 'small',
        'unsigned' => TRUE,
      ),

      'building_dashboard' => array(
        'type' => 'varchar',
        'length' => 255,
      ),

      'teacher_dashboard' => array(
        'type' => 'varchar',
        'length' => 255,
      ),
    ),

    'primary key' => array('bldg_id'),
  );

  // SChool year
  $schema['i_school_years'] = array(
    'description' => 'School years for statistics',
    'fields' => array(
      'school_year' => array(
        'type' => 'int',
        'not null' => TRUE,
      ),
      'label' => array(
        'type' => 'varchar',
        'not null' => TRUE,
        'length' => 60,
      ),
      'start_date' => array(
         'type' => 'date',
         'mysql_type' => 'DATE',
         'pgsql_type' => 'DATE',
         'sortable' => TRUE,
      ),
      'end_date' => array(
         'type' => 'date',
         'mysql_type' => 'DATE',
         'pgsql_type' => 'DATE',
         'sortable' => TRUE,
      ),
    ),
    'primary_key' => array('school_year'),
  );

  //Grade levels
  $schema['i_grade_levels'] = array(
    'description' => 'Grade level validation',
    'fields' => array(
      'grade_level' => array(
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
      ),
      'name' => array(
        'type' => 'varchar',
        'length' => 60,
      ),
      'abbrev' => array(
        'type' => 'varchar',
        'length' => 25,
      ),
    ),
    'primary key' => array('grade_level'),
  );

  // People
  $schema['p_people'] = array(
    'description' => 'Every person, including teachers, students and administrators. ',
    'fields' => array(

      'person_id' => array(
        'description' => 'Unique key for person',
        'type' => 'serial',
        'size' => 'big',
        'not null' => TRUE,
        'unsigned' => TRUE,
      ),

      'sis_id' => array(
        'description' => 'External displayable unique id for students',
        'type' => 'varchar',
        'length' => 30,
      ),

      'login' => array(
        'type' => 'varchar',
        'length' => 60,
      ),

      'last_name' => array(
        'type' => 'varchar',
        'length' => 60,
        'not null' => TRUE,
      ),

      'first_name' => array(
        'type' => 'varchar',
        'length' => 60,
      ),

      'middle_name' => array(
        'type' => 'varchar',
        'length' => 60,
      ),

      'email' => array(
        'type' => 'varchar',
        'length' => 150,
      ),

      'address_street' => array(
        'type' => 'varchar',
        'length' => 75,
      ),

      'address_city' => array(
        'type' => 'varchar',
        'length' => 75,
      ),

      'address_state' => array(
        'type' => 'varchar',
        'length' => 75,
      ),

      'address_zip' => array(
        'type' => 'varchar',
        'length' => 10,
      ),

      'gender' => array(
        'type' => 'varchar',
        'length' => 1,
      ),

      'birthdate' => array(
         'type' => 'date',
         'mysql_type' => 'DATE',
         'pgsql_type' => 'DATE',
         'sortable' => TRUE,
      ),

      'last_modified' => array(
        'type' => 'datetime',
        'mysql_type' => 'DATETIME',
        'pgsql_type' => 'timestamp without time zone',
        'sqlite_type' => 'VARCHAR',
        'sqlsrv_type' => 'smalldatetime',
        'sortable' => TRUE,
      ),
    ),
    'primary key' => array('person_id'),
  );

  // Students (enrollment)
  $schema['p_students'] = array(
    'description' => 'Student enrollment',
    'fields' => array(

      'person_id' => array(
        'type' => 'int',
        'size' => 'big',
        'not null' => TRUE,
        'unsigned' => TRUE,
      ),

      'student_id' => array(
        'type' => 'serial',
        'size' => 'big',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),

      'bldg_id' => array(
        'description' => 'Building enrolled in',
        'type' => 'int',
        'not null' => TRUE,
      ),

      'school_year' => array(
        'type' => 'int',
        'size' => 'small',
        'description' => 'School Year of enrollment',
      ),

      'grade_level' => array(
        'description' => 'Grade level enrolled',
        'type' => 'int',
        'not null' => TRUE,
        'unsigned' => TRUE,
      ),
    ),
    'primary key' => array('student_id'),
  );

  //Student attributes
  $schema['p_student_attributes'] = array(
    'description' => 'Attributes for student records.',
    'fields' => array(

      'student_id' => array(
        'description' => 'Id to student enrollment record',
        'type' => 'int',
        'size' => 'big',
        'unsigned' => TRUE,
        'not null' => TRUE,
        ),

      'attribute' => array(
        'description' => 'attribute',
        'type' => 'varchar',
        'length' => 60,
        'not null' => TRUE,
      ),
    ),

    'primary key' => array('student_id', 'attribute')
  );

  // Student attribute validatioins
  $schema['i_student_attributes'] = array(
    'description' => 'Student attribute validation',
    'fields' => array(

      'attribute' => array(
        'description' => 'Student Attribute value',
        'type' => 'varchar',
        'length' => 60,
        'not null',
      ),

      'category' => array(
        'description' => 'Category that student attribute falls in',
        'type' => 'varchar',
        'length' => 60,
        'not null' => TRUE,
      ),
    ),

    'primary key' => array('attribute'),
  );

  // Student groups
  $schema['s_groups'] = array(
    'description' => 'Groups of students (e.g. courses)',
    'fields' => array(

      'group_id' => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'size' => 'big',
        'not null' => TRUE,
      ),

      'category' => array(
        'type' => 'varchar',
        'length' => 60,
      ),

      'name' => array(
        'type' => 'varchar',
        'length' => 128,
        'not null' => TRUE,
      ),

      'bldg_id' => array(
        'type' => 'int',
       ),

       'school_year' => array(
         'type' => 'int',
         'size' => 'small',
       ),

       'group_type' => array(
         'type' => 'varchar',
         'length' => 25,
         'not null' => TRUE,
         'default' => 'course'
       ),

       'min_grade_level' => array(
         'type' => 'int',
         'size' => 'tiny',
       ),

       'max_grade_level' => array(
         'type' => 'int',
         'size' => 'tiny',
       ),

       'code' => array(
         'type' => 'varchar',
         'length' => 25,
       ),

       'owner_id' => array(
         'type' => 'int',
       ),

    ),
    'primary key' => array('group_id'),
  );

  $schema['s_group_member'] = array(
    'description' => 'Membership of group data',
    'fields' => array(
      'group_id' => array(
        'type' => 'int',
        'size' => 'big',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),

      'student_id' => array(
        'type' => 'int',
        'not null' => TRUE,
      ),

    ),

    'primary key' => array('group_id', 'student_id'),

  );

  $schema['p_staff'] = array(
    'description' => 'Staff of a building and their roles',
    'fields' => array(
       'staff_id' => array(
         'type' => 'serial',
         'size' => 'big',
         'unsigned' => 'true',
         'not null' => TRUE,
       ),

       'bldg_id' => array(
         'type' => 'int',
         'not null' => TRUE,
       ),

       'person_id' => array(
         'type' => 'int',
         'size' => 'big',
         'unsigned' => TRUE,
         'not null' => TRUE,
       ),

       'min_grade_level' => array(
         'type' => 'int',
         'size' => 'tiny',
       ),

       'max_grade_level' => array(
         'type' => 'int',
         'size' => 'tiny',
       ),

       'role' => array(
         'type' => 'varchar',
         'length' => 25,
       ),
    ),
    'primary key' => array('staff_id')
  );

  $schema['a_tests'] = array(
    'description' => 'Test Definitions',
    'fields' => array(
      'test_id' => array(
        'type' => 'serial',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'name' => array(
        'type' => 'varchar',
        'length' => 75,
        'not null' => TRUE,
      ),
      'abbrev' => array(
        'type' => 'varchar',
        'length' => 25,
      ),
      'code' => array(
        'type' => 'varchar',
        'length' => 50,
      ),
      'subject_area' => array(
        'type' => 'varchar',
        'length' => 25,
      ),
      'inactive' => array(
        'type' => 'int',
      ),
      'min_grade' => array(
        'type' => 'int',
        'size' => 'tiny'
      ),
      'max_grade' => array(
        'type' => 'int',
        'size' => 'tiny'
      ),
      'weight' => array(
        'type' => 'int',
      ),
      'allow_data_entry' => array(
        'type' => 'int',
      ),
    ),
    'primary key' => array('test_id'),
  );

  $schema['a_test_measures'] = array(
    'description' => 'Measure Definition',
    'fields' => array(
      'measure_id' => array(
        'type' => 'serial',
      ),
      'test_id' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'name' => array(
        'type' => 'varchar',
        'length' => 75,
      ),
      'abbrev' => array(
        'type' => 'varchar',
        'length' => 25,
      ),
      'code' => array(
        'type' => 'varchar',
        'length' => 128,
      ),
      'description' => array(
        'type' => 'varchar',
        'length' => 400,
      ),
      'parent_measure' => array(
        'type' => 'int',
      ),
      'calc_rule' => array(
        'type' => 'varchar',
        'length' => 75,
      ),
      'subject' => array(
        'type' => 'varchar',
        'length' => 60,
      ),
      'grad_requirement' => array(
        'type' => 'varchar',
        'length' => 60,
      ),
    ),
    'primary key' => array('measure_id'),
  );

  $schema['a_test_measure_calcs'] = array(
    'description' => 'List of measures in the calc',
    'fields' => array(
      'measure_id' => array(
        'type' => 'int',
        'not null' => TRUE,
        'unsigned' => TRUE,
       ),
      'calc_measure_id' => array(
         'type' => 'int',
         'unsigned' => TRUE,
         'not null' => TRUE,
       ),
    ),
    'primary key' => array('measure_id', 'calc_measure_id'),
  );

  $schema['a_test_schedules'] = array(
    'description' => 'Schedules',
    'fields' => array(
      'test_id' => array(
        'type' => 'int',
        'not null' => TRUE,
      ),
      'seq' => array(
        'type' => 'int',
        'size' => 'tiny',
      ),
      'label' => array(
        'type' => 'varchar',
        'length' => 255,
      ),
      'start_day' => array(
        'type' => 'int',
        'default' => 0,
      ),
      'end_day'=> array(
        'type' => 'int',
        'default' => 300,
      ),
      'target_day' => array(
        'type' => 'int',
        'default' => 150,
      ),
    ),
    'primary key' => array('test_id', 'seq'),
  );

  $schema['a_test_rules'] = array(
    'description' => 'Normalization rules',
    'fields' => array(
      'measure_id' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'grade_level' => array(
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
      ),
      'seq' => array(
        'type' => 'int',
        'size' => 'tiny',
        'not null' => TRUE,
      ),
      'level_1' => array(
        'type' => 'numeric',
        'precision' => 6,
        'scale' => 2,
      ),
      'level_2' => array(
        'type' => 'numeric',
        'precision' => 6,
        'scale' => 2,
      ),
      'level_3' => array(
        'type' => 'numeric',
        'precision' => 6,
        'scale' => 2,
      ),
      'level_4' => array(
        'type' => 'numeric',
        'precision' => 6,
        'scale' => 2,
      ),
      'max_score' => array(
        'type' => 'numeric',
        'precision' => 6,
        'scale' => 2,
      ),
    ),
    'primary key' => array('measure_id', 'grade_level', 'seq'),
  );

  $schema['a_assessments'] = array(
    'description' => 'Individual assessments',
    'fields' => array(
      'assessment_id' => array(
        'type' => 'serial',
        'size' => 'big',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'test_id' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'person_id' => array(
        'type' => 'int',
        'size' => 'big',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'grade_level' => array(
        'type' => 'int',
        'size' => 'tiny',
      ),
      'school_year' => array(
        'type' => 'int',
        'size' => 'small',
      ),
      'bldg_id' => array(
        'type' => 'int',
        'size' => 'small',
      ),
      'date_taken' => array(
         'type' => 'date',
         'mysql_type' => 'DATE',
         'pgsql_type' => 'DATE',
         'sortable' => TRUE,
      ),
      'attr1' => array(
        'type' => 'varchar',
        'length' => 30,
      ),
      'attr2' => array(
        'type' => 'varchar',
        'length' => 30,
      ),
      'comments' => array(
        'type' => 'text',
      ),
      'modified' => array(
         'type' => 'date',
         'mysql_type' => 'DATE',
         'pgsql_type' => 'DATE',
         'sortable' => TRUE,
      ),
      'source' => array(
        'type' => 'varchar',
        'length' => 30,
      ),
      'modified_by' => array(
        'type' => 'varchar',
        'length' => 60,
      ),
    ),
    'primary key' => array('assessment_id'),
  );

  $schema['a_scores'] = array(
    'description' => 'Test scores for each assessment',
    'fields' => array(
      'assessment_id' => array(
        'type' => 'int',
        'size' => 'big',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),
      'measure_id' => array(
         'type' => 'int',
         'unsigned' => TRUE,
         'not null' => TRUE,
      ),
      'score' => array(
        'type' => 'numeric',
        'precision' => 6,
        'scale' => 2,
        'not null' => TRUE,
      ),
      'norm_score' => array(
        'type' => 'numeric',
        'precision' => 6,
        'scale' => 2,
      ),
    ),
    'primary key' => array('assessment_id', 'measure_id'),
  );

  $schema['a_profiles'] = array(
    'description' => 'Test Profiles for displaying measures',
    'fields' => array(
      'profile_id' => array(
        'type' => 'serial',
        'size' => 'big',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),

      'name' => array(
        'type' => 'varchar',
        'length' => 75,
      ),

      'min_grade' => array(
        'type' => 'int',
        'size' => 'tiny',
      ),

      'max_grade' => array(
        'type' => 'int',
        'size' => 'tiny',
      ),

      'bldg_id' => array(
        'type' => 'int',
        'not null' => TRUE,
        'default' => 0,
      ),

      'weight' => array(
        'type' => 'int',
      ),

      'school_year_offset' => array(
        'type' => 'int',
        'default' => 0,
      ),


    ),
    'primary key' => array('profile_id'),
  );

  $schema['a_profile_measures'] = array(
    'description' => 'Test Profile measures to display',
    'fields' => array(

      'profile_id' => array(
        'type' => 'int',
        'size' => 'big',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),

      'sort_order' => array(
        'type' => 'int',
        'not null' => TRUE,
      ),

      'measure_id' => array(
        'type' => 'int',
        'unsigned' => TRUE,
        'not null' => TRUE,
      ),

      'seq' => array(
        'type' => 'int',
      ),

      'label' => array(
        'type' => 'varchar',
        'length' => 30,
      ),
    ),
    'primary key' => array('profile_id', 'sort_order'),
  );

  $schema['a_profile_displays'] = array(
    'description' => 'Test Profile displays',
    'fields' => array(
      'profile_id' => array(
        'type' => 'int',
        'size' => 'big',
        'unsigned' => TRUE,
      ),

      'display' => array(
        'type' => 'varchar',
        'length' => 30,
      ),
    ),
    'primary key' => array('profile_id', 'display'),
  );

  return $schema;
}
