--ACCESS=teacher
SELECT p.*, a_profile_measures_xml(p.profile_id) AS measures from a_profiles p
WHERE profile_id=:profile_id
ORDER BY p.weight, p.min_grade, p.max_grade, p.max_grade, p.name