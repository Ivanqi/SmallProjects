<?php
namespace Core\Http;

use Core\Support\ServiceProvider;
use Core\Routing\Router;

class RoutingServiceProvider extends ServiceProvider {
    public function register()
    {
        $this->registerRouter();
    }

    protected function registerRouter()
    {
        $this->app->singleton('router', function ($app) {
            return new Router($app);
        });
    }
}