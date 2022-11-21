<?php
namespace Core;

use Core\Routing\Router;

class Kernel {
    protected $app;
    protected $router;
    protected $middlewareGroups = [];

    protected $bootstrappers = [
        \Core\Bootstrap\LoadConfiguration::class,
    ];

    public function __construct(Application $app, Router $router)
    {
        $this->app = $app;
        $this->router = $router;

        $router->middlewarePriority = $this->middlewarePriority;
    }

    protected function bootstrappers()
    {
        return $this->bootstrappers;
    }

    public function bootstrap()
    {
        $this->app->bootstrapWith($this->bootstrappers());
    }

    public function handle($request)
    {
        $this->bootstrap();
        $this->dispatchToRouter($request);
    }

    public function dispatchToRouter($request)
    {
        $this->router->get('/user/{id}/', 'UsersController@index');
        // $router->get('/user/{id}/', function () {
        //     echo "aaaa";
        // });
        $this->router->dispatch($request);
    }
}