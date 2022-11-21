<?php
namespace Core\Routing;

class BaseRoute {

    private $path = '/';
    private $host = '';
    private $schemes = [];
    private $methods = [];
    private $defaults = [];
    private $requirements = [];
    private $options = [];
    private $condition = '';

    private $compiled;

    public function __construct($path, array $defaults = [], array $requirements = [], array $options = [], $host = '', $schemes = [], $methods = [], $condition = '')
    {
        $this->setPath($path);
        $this->setDefaults($defaults);
        $this->setRequirements($requirements);
        $this->setOptions($options);
        $this->setHost($host);
        $this->setSchemes($schemes);
        $this->setMethods($methods);
        $this->setCondition($condition);
    }

    public function setPath($pattern)
    {
        $this->path = '/'.ltrim(trim($pattern), '/');
        $this->compiled = null;

        return $this;
    }

    public function setHost($pattern)
    {
        $this->host = (string) $pattern;
        $this->compiled = null;

        return $this;
    }

    public function setSchemes($schemes)
    {
        $this->schemes = array_map('strtolower', (array) $schemes);
        $this->compiled = null;

        return $this;
    }

    public function setMethods($methods)
    {
        $this->methods = array_map('strtoupper', (array) $methods);
        $this->compiled = null;

        return $this;
    }

    public function setOptions(array $options)
    {
        $this->options = [
            'compiler_class' => 'Core\\Routing\\BaseRouteCompiler',
        ];

        return $this->addOptions($options);
    }

    public function addOptions(array $options)
    {
        foreach ($options as $name => $option) {
            $this->options[$name] = $option;
        }
        $this->compiled = null;

        return $this;
    }

    public function setDefaults(array $defaults)
    {
        $this->defaults = array();

        return $this->addDefaults($defaults);
    }

    public function addDefaults(array $defaults)
    {
        foreach ($defaults as $name => $default) {
            $this->defaults[$name] = $default;
        }
        $this->compiled = null;

        return $this;
    }

    public function setCondition($condition)
    {
        $this->condition = (string) $condition;
        $this->compiled = null;

        return $this;
    }

    public function getOption($name)
    {
        return isset($this->options[$name]) ? $this->options[$name] : null;
    }

    public function getHost()
    {
        return $this->host;
    }

    public function getPath()
    {
        return $this->path;
    }

    public function setRequirements(array $requirements)
    {
        $this->requirements = [];

        return $this->addRequirements($requirements);
    }

    public function addRequirements(array $requirements)
    {
        foreach ($requirements as $key => $regex) {
            $this->requirements[$key] = $this->sanitizeRequirement($key, $regex);
        }
        $this->compiled = null;

        return $this;
    }

    private function sanitizeRequirement($key, $regex)
    {
        if (!is_string($regex)) {
            throw new \Exception(sprintf('Routing requirement for "%s" must be a string.', $key));
        }

        if ('' !== $regex && '^' === $regex[0]) {
            $regex = (string) substr($regex, 1); // returns false for a single character
        }

        if ('$' === substr($regex, -1)) {
            $regex = substr($regex, 0, -1);
        }

        if ('' === $regex) {
            throw new \Exception(sprintf('Routing requirement for "%s" cannot be empty.', $key));
        }

        return $regex;
    }

    public function getRequirements()
    {
        return $this->requirements;
    }

    public function getRequirement($key)
    {
        return isset($this->requirements[$key]) ? $this->requirements[$key] : null;
    }

    public function compile()
    {
        if (null !== $this->compiled) {
            return $this->compiled;
        }

        $class = $this->getOption('compiler_class');

        return $this->compiled = $class::compile($this);
    }
    
}