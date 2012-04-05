--ACCESS=teacher
SELECT p.*, s.bldg_id, s.student_id, s.grade_level FROM
  p_students s JOIN p_people p ON s.person_id=p.person_id 
  WHERE school_year = COALESCE(:school_year,i_school_year()) 