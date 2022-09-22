<?php
namespace Core\Support;

abstract class ServiceProvider {
    protected $app;

    public function __construct($app)
    {
        $this->app = $app;
    }
}