<?php
/*
 * Sample Repository configuration file
 */

/*
 * Security provider:  Specify the class name that is used to provide security
 */
$conf['access callback'] = 'user_access';
$conf['user callback'] = 'forena_current_user_name';
/*
 * Data provider:
 * Specify the class name that will be used to interpret data block files.
 * Note that data blocks in a repository
 *
 */
$conf['data_engine'] = 'FrxPostgres';
$conf['postgres_xml'] = TRUE;

/*
 * URI:
 * The format of the uri depends on the type of data engine that's being used.
 * In database engines it might be the connection string to the db.  In the file
 * engine it would be the path to the directory containting the files
 */
$conf['debug'] = @$_GET['debug'];

