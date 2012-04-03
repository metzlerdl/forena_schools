--ACCESS=sys_admin 
select i.measure_code,count(m.measure_id) AS measres, count(distinct p.person_id) as people from 
  imp_test_scores i JOIN a_tests t ON i.test_code=t.code 
  LEFT JOIN a_test_measures m ON m.test_id=t.test_id AND i.measure_code=m.code
  LEFT JOIN p_people p ON p.sis_id=i.sis_id
  group by i.measure_code