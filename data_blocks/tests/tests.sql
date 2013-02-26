--ACCESS=PUBLIC
SELECT t.*, gmin.abbrev min_grade_label, gmax.abbrev max_grade_label from a_tests t
  JOIN i_grade_levels gmin ON t.min_grade=gmin.grade_level
  JOIN i_grade_levels gmax ON t.max_grade=gmax.grade_level
--IF=:grade_level
WHERE :grade_level BETWEEN min_grade AND max_grade
ORDER BY t.name