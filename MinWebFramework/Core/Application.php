<?php
namespace Core;

use Core\Container\Container;
use Core\Http\RoutingServiceProvider;

class Application extends Container {

    const VERSION = '0.0.1';

    protected $basePath;

    public function __construct($basePath = null)
    {
        if ($basePath) {
            $this->setBasePath($basePath);
        }

        $this->registerBaseBindings();

        $this->registerBaseServiceProviders();
    }

    public function registerBaseBindings()
    {
        static::setInstance($this);
    }

    public function setBasePath($basePath)
    {
        $this->basePath = rtrim($basePath, '\/');
    }

    protected function registerBaseServiceProviders()
    {
        $this->register(new RoutingServiceProvider($this));
    }

    public function register($provider)
    {   
        if (is_string($provider)) {
            return $provider;
        }

        if (method_exists($provider, 'register')) {
            $provider->register();
        }

        return $provider;
    }

    public function dispatchToRouter($request)
    {
        $router = self::getInstance()->make('router');
        $router->get('/user/{id}/', 'UsersController@index');
        $router->dispatch($request);
    }
}