--ACCESS=administer permissions
SELECT * FROM {role_permission} p
  WHERE p.rid = :role 