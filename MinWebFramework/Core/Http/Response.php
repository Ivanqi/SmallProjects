<?php
namespace Core\Http;

class Response extends BaseResponse {
    public function setContent($content)
    {
        $this->original = $content;

        // If the content is "JSONable" we will set the appropriate header and convert
        // the content to JSON. This is useful when returning something like models
        // from routes that will be automatically transformed to their JSON form.
        // if ($this->shouldBeJson($content)) {
        //     $this->header('Content-Type', 'application/json');

        //     $content = $this->morphToJson($content);
        // }

        // If this content implements the "Renderable" interface then we will call the
        // render method on the object so we will avoid any "__toString" exceptions
        // that might be thrown and have their errors obscured by PHP's handling.
        // elseif ($content instanceof Renderable) {
        //     $content = $content->render();
        // }

        return parent::setContent($content);
    }
}