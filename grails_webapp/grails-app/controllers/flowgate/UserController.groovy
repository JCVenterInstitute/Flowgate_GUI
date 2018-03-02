package flowgate

import grails.plugin.springsecurity.annotation.Secured

@Secured(['ROLE_Admin','ROLE_Administrator'])
class UserController {

    def springSecurityService

    def axActivateUser(){
        User userToActivate = User.findById(params?.uId)
//        TODO move to service!
        if(userToActivate?.authorities.find {it == flowgate.Role.findByAuthority('ROLE_NewUser')}){
            UserRole.remove userToActivate, Role.findByAuthority('ROLE_NewUser')
            UserRole.remove userToActivate, Role.findByAuthority('ROLE_Guest')
            UserRole.create userToActivate, Role.findByAuthority('ROLE_User')
        }
        else {
//            userToActivate.authorities.each {
            UserRole.removeAll userToActivate
//            }
            UserRole.create userToActivate, Role.findByAuthority('ROLE_Guest')
            UserRole.create userToActivate, Role.findByAuthority('ROLE_NewUser')
        }
//        TODO send mail to user saying that she/he is activated
        render (contentType:"text/json"){
            success true
            activatedField "${g.render(template: 'activateField', model: [user: userToActivate])}"
        }

    }

    def newUsers() {
        def newUsers = UserRole.findAllByRole(Role.findByAuthority('ROLE_NewUser')).user
//        def newUsers = User.list()
        newUsers += UserRole.findAllByRole(Role.findByAuthority('ROLE_User')).user
        [newUserLst: newUsers, userCount: User.count()]
    }

    def index() {
        respond User.list(), model:[userCount: User.count()]
    }

    def show() {}

    def update(User user) {}

    def Logout(){
        log.info "user agent " + request.getHeader("User-Agent")
//        session.invalidate()
        redirect action: 'login'
    }

    def login() {
//        TODO get settings from db
        request.getSession(true)
    }
}
