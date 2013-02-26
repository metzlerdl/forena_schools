--ACCESS=teacher
SELECT :bldg_id as bldg_id, :grade_level as grade_level, profile_id, name,  a_profile_measures_xml(profile_id) AS measures from a_profiles p
        WHERE (bldg_id=:bldg_id OR bldg_id=-1) AND :grade_level BETWEEN min_grade and max_grade
          AND analysis_only = false
        ORDER BY p.weight, p.min_grade, p.max_grade, p.name