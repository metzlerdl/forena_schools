--ACCESS=PUBLIC
SELECT distinct grad_requirement AS subject, grad_requirement as name from a_test_measures 
  WHERE grad_requirement IS NOT NULL
  