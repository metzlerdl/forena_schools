--ACCESS=PUBLIC
SELECT bldg_id, name, abbrev, code, sis_code 
from i_buildings 
WHERE bldg_id in (:security.buildings)
  order by case when bldg_id=-1 then 0 else 1 end, name