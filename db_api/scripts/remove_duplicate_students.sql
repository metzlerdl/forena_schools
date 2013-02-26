delete from p_people where person_id in (select person_id from (select rv.*,
 row_number() over (partition by sis_id order by new_tests desc, total_tests desc, person_id) as r
  from (select p.person_id, p.sis_id, p.last_name, p.first_name, p.login,
  (select count(1) from p_students s join s_group_members m on s.student_id=m.student_id where s.PERSON_ID=p.PERSON_ID and s.school_year=2012) courses, 
  (select count(1) from a_assessments a WHERE a.person_id=p.person_id and a.school_year=2012) new_tests,
  (select count(1) from a_assessments a WHERE a.person_id=p.person_id) total_tests
  from p_people 
  p where sis_id in (select sis_id from p_people group by sis_id having count(1)>1)
  ) rv
) d where r>1); 
delete from a_assessments a where not exists (select 1 from p_people p where a.person_id=p.person_id); 
delete from p_students s where not exists (select 1 from p_people p where s.person_id=p.person_id); 
delete from s_group_members m where NOT EXISTS(SELECT 1 from p_students s where s.student_id=m.student_id); 
  
  
 