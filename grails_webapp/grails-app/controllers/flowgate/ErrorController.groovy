package flowgate

class ErrorController {

    def index() {}

    def unauthorized() {
        flash.error = "Please login to continue."
        render view: "/index"
    }
}
