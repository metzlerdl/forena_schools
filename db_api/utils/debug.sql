CREATE OR REPLACE FUNCTION debug(p_title VARCHAR, p_message VARCHAR) RETURNS VOID AS 
$$
BEGIN 
  INSERT INTO logs(msg_type, title, message) VALUES ('debug',p_title, p_message); 
  END; 
$$ LANGUAGE plpgsql; 