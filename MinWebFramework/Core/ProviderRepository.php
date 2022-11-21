<?php
namespace Core;

class ProviderRepository {

    protected $app;

    public function __construct($app)
    {
        $this->app = $app;
    }

    public function createProvider($provider)
    {
        return new $provider($this->app);
    }

    protected function freshManifest(array $providers)
    {
        return ['providers' => $providers, 'eager' => [], 'deferred' => []];
    }

    protected function compileManifest($providers)
    {
        $manifest = $this->freshManifest($providers);

        foreach ($providers as $provider) {
            $instance = $this->createProvider($provider);
        }

        return $manifest;
    }

    public function load(array $providers)
    {
        $manifest = $this->compileManifest($providers);
    }
}