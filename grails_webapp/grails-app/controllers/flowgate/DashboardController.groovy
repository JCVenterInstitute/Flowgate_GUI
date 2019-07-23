package flowgate

import grails.plugin.springsecurity.annotation.Secured


@Secured(["ROLE_Administrator","ROLE_Admin"])
class DashboardController {

    def index() { }
}
