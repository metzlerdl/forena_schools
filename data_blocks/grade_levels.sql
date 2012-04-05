--ACCESS=PUBLIC
select 
  grade_level,
  name AS grade_level_name,
  case when :grade_level=grade_level THEN 'selected' END AS class 
from i_grade_levels l
  order by grade_level