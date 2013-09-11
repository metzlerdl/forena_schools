--ACCESS=teacher
SELECT y.school_year, y.label as year_label, :bldg_id AS bldg_id, :grade_level AS grade_level, p.profile_id, name,  a_profile_measures_xml(p.profile_id) AS measures
  from a_profiles p 
  JOIN i_school_years y ON y.school_year = COALESCE(:school_year, i_school_year()) 
  JOIN a_profile_displays d ON p.profile_id=d.profile_id
        WHERE (bldg_id=:bldg_id OR bldg_id=-1) AND :grade_level BETWEEN min_grade and max_grade
          AND analysis_only = false
          AND d.display = 'Grade Level'
          AND p.profile_id=COALESCE(:profile_id, p.profile_id)
        ORDER BY p.weight, p.min_grade, p.max_grade, p.name
        LIMIT 1