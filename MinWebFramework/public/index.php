<?php
require __DIR__.'/../bootstrap/autoload.php';

$app = new Core\Application(
    realpath(__DIR__.'/../')
);


try {
    $request = Core\Http\Request::capture();
    $app->dispatchToRouter($request);
    $response = Core\Http\Response();
    $response->send();
} catch(\Exception $e) {
    print_r([$e->getMessage()]);
}



