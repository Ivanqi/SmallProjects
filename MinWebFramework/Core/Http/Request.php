<?php
namespace Core\Http;

class Request extends BaseRequest {

    public static function capture()
    {
        // static::enableHttpMethodParameterOverride();

        return static::createFromBase(BaseRequest::createFromGlobals());
    }

    public static function createFromBase(BaseRequest $request)
    {
        if ($request instanceof static) {
            return $request;
        }

        $content = $request->content;

        $request = (new static)->duplicate(
            $request->query->all(), $request->request->all(), $request->attributes->all(),
            $request->cookies->all(), $request->files->all(), $request->server->all()
        );

        $request->content = $content;

        $request->request = $request->getInputSource();

        return $request;
    }
}