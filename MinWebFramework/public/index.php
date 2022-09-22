<?php
require __DIR__.'/../bootstrap/autoload.php';

$app = new Core\Application(
    realpath(__DIR__.'/../')
);

$request = Core\Http\Request::capture();
$app->dispatchToRouter($request);
// $response = Core\Http\Response();

