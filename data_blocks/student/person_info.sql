--ACCESS=teacher
SELECT 
  p.*
 FROM p_people p 
 WHERE person_id = :person_id
 