--ACCESS=teacher
SELECT t.test_id, t.name FROM i_buildings b 
  JOIN a_tests t ON 
    (t.min_grade between b.min_grade and b.max_grade)
    OR (t.max_grade between b.min_grade AND b.max_grade)
    OR (b.min_grade<t.min_grade AND t.max_grade<=b.max_grade)
where bldg_id=:bldg_id
  AND bldg_id IN (:security.buildings)
  AND inactive='0'
--IF=:grade_level
  AND :grade_level BETWEEN t.min_grade AND t.max_grade
--END
ORDER BY t.name