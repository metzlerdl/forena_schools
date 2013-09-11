--ACCESS=PUBLIC
select 
  l.grade_level,
  l.name AS grade_level_name,
  case when :grade_level=l.grade_level THEN 'selected' END AS class 
from i_grade_levels l
--IF=:bldg_id
  JOIN i_buildings b on l.grade_level between b.min_grade AND b.max_grade
    AND b.bldg_id=:bldg_id
--END
  where grade_level in (:security.grades)
  order by grade_level