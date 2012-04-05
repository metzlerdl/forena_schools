--ACCESS=PUBLIC
select m.*,
 CASE WHEN :subject=subject THEN 'selected' END selected
 from (SELECT DISTINCT subject from a_test_measures) m
ORDER BY 1