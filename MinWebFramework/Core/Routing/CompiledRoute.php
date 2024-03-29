<?php
namespace Core\Routing;

class CompiledRoute {
    private $variables;
    private $tokens;
    private $staticPrefix;
    private $regex;
    private $pathVariables;
    private $hostVariables;
    private $hostRegex;
    private $hostTokens;

    public function __construct($staticPrefix, $regex, array $tokens, array $pathVariables, $hostRegex = null, array $hostTokens = [], array $hostVariables = [], array $variables = [])
    {
        $this->staticPrefix = (string) $staticPrefix;
        $this->regex = $regex;
        $this->tokens = $tokens;
        $this->pathVariables = $pathVariables;
        $this->hostRegex = $hostRegex;
        $this->hostTokens = $hostTokens;
        $this->hostVariables = $hostVariables;
        $this->variables = $variables;
    }

    public function serialize()
    {
        return serialize(array(
            'vars' => $this->variables,
            'path_prefix' => $this->staticPrefix,
            'path_regex' => $this->regex,
            'path_tokens' => $this->tokens,
            'path_vars' => $this->pathVariables,
            'host_regex' => $this->hostRegex,
            'host_tokens' => $this->hostTokens,
            'host_vars' => $this->hostVariables,
        ));
    }

    public function unserialize($serialized)
    {
        if (\PHP_VERSION_ID >= 70000) {
            $data = unserialize($serialized, array('allowed_classes' => false));
        } else {
            $data = unserialize($serialized);
        }

        $this->variables = $data['vars'];
        $this->staticPrefix = $data['path_prefix'];
        $this->regex = $data['path_regex'];
        $this->tokens = $data['path_tokens'];
        $this->pathVariables = $data['path_vars'];
        $this->hostRegex = $data['host_regex'];
        $this->hostTokens = $data['host_tokens'];
        $this->hostVariables = $data['host_vars'];
    }

    public function getStaticPrefix()
    {
        return $this->staticPrefix;
    }

    public function getRegex()
    {
        return $this->regex;
    }

    public function getHostRegex()
    {
        return $this->hostRegex;
    }

    public function getTokens()
    {
        return $this->tokens;
    }

    public function getHostTokens()
    {
        return $this->hostTokens;
    }

    public function getVariables()
    {
        return $this->variables;
    }

    public function getPathVariables()
    {
        return $this->pathVariables;
    }

    public function getHostVariables()
    {
        return $this->hostVariables;
    }
}