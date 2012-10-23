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
  
  // People 
  $schema['p_people'] = array(
    'description' => 'Every person, including teachers, students and administrators. ', 
    'fields' => array(
  
      'person_id' => array(
        'description' => 'Unique key for person', 
        'type' => 'serial', 
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
        'not null' => TRUE, 
        'unsigned' => TRUE, 
      ), 
      
      'student_id' => array(
        'type' => 'serial', 
        'not null' => TRUE,
      ),
      
      'bldg_id' => array(
        'description' => 'Building enrolled in', 
        'type' => 'int', 
        'not null' => TRUE, 
      ), 
      
      'school_year' => array(
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
       ), 
       
       'group_type' => array(
         'type' => 'varchar', 
         'length' => 25, 
         'not null' => TRUE, 
         'default' => 'course'
       ), 
       
       'min_grade_level' => array(
         'type' => 'int',
         'size' => 'small', 
       ), 
       
       'max_grade_level' => array(
         'type' => 'int', 
         'size' => 'small', 
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
        'not null' => TRUE, 
      ), 
      
      'student_id' => array(
        'type' => 'int', 
        'not null' => TRUE, 
      ), 
      
    ), 
    
    'primary key' => array('group_id', 'student_id'), 
    
  ); 
  
  return $schema; 
}
