<?php

define('HASH', md5(time()));
define('PATH_TMP', '../upload/' . HASH . '.jpg');
define('PATH_OUTPUT', '../upload/' . HASH . '.bmp');
define('PATH_FRAME', '../frame/*.png');

if (!move_uploaded_file($_FILES['image']['tmp_name'], PATH_TMP)) {
	echo 0;
	exit;
}

$frames = glob(PATH_FRAME);
shuffle($frames);

$cmd_resize = '\( ' . PATH_TMP . ' -resize "683x384^" -gravity center -crop 683x384+0+0 \)';
$cmd_composite = '\( ' . $cmd_resize . ' ' . $frames[0] . ' -composite \)';
$cmd_normalize = 'convert ' . $cmd_composite . ' -rotate 90 -colorspace gray -dither FloydSteinberg -colors 2 -normalize -colorspace RGB ' . PATH_OUTPUT;
exec($cmd_normalize);
echo 1;

