--ACCESS=PUBLIC
SELECT m.*, t.name as test_name, t.abbrev as test_abbrev, gmin.abbrev min_grade_label, gmax.abbrev max_grade_label,
  (SELECT max(greatest(level_1,level_2, level_3, level_4, max_score)) as max_value from a_test_rules r WHERE r.measure_id=m.measure_id ) max_value
FROM 
  a_test_measures m JOIN a_tests t ON m.test_id=t.test_id
  JOIN i_grade_levels gmin ON t.min_grade=gmin.grade_level
  JOIN i_grade_levels gmax ON t.max_grade=gmax.grade_level
  WHERE measure_id = :measure_id