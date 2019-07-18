package flowgate

import grails.plugin.springsecurity.annotation.Secured
import grails.transaction.Transactional

@Secured(['ROLE_Admin','ROLE_Administrator'])
class UserController {

    def customUserDetailsService

    def axActivateUser(){
        User userToActivate = User.findById(params?.uId)
//        TODO move to service!
        if(userToActivate?.authorities.find {it == flowgate.Role.findByAuthority('ROLE_NewUser')}){
            customUserDetailsService(userToActivate, true)
        }
        else {
            customUserDetailsService(userToActivate, false)
        }
//        TODO send mail to user saying that she/he is activated
        render (contentType:"text/json"){
            success true
            activatedField "${g.render(template: 'activateField', model: [user: userToActivate])}"
        }

    }

    def list() {
        //def newUsers = UserRole.findAllByRole(Role.findByAuthority('ROLE_NewUser')).user
        def newUsers = User.list()
        //newUsers += UserRole.findAllByRole(Role.findByAuthority('ROLE_User')).user
        [newUserLst: newUsers, userCount: User.count()]
    }

    def index() {
        respond User.list(), model:[userCount: User.count()]
    }

    def create() {
        respond new User(params)
    }

    def save(User user) {
        if (user == null) {
            notFound()
            return
        }

        if (user.hasErrors()) {
            //transactionStatus.setRollbackOnly()
            respond user.errors, view:'create', model:[user: user]
            return
        }

        customUserDetailsService.createUser(user)

        flash.message = "User " + user.username + " created!"

        redirect action: 'list'
    }

    def edit() {
        def user = User.findById(params?.id);
        [user: user]
    }

    @Transactional
    def update(User user) {
        boolean valid = user.validate()
        if(valid) {
            user.save flush:true

            customUserDetailsService.activateUser(user, user.enabled)
            flash.success = "User has been updated!"
            redirect action: 'edit', params: [id: user.id]
        } else {
            render view: 'edit', model: [user: user]
        }
    }

    def updatePassword(User user) {
        String oldPass = params?.oldpass
        String newPass = params?.newpass
        String confirmPass = params?.confirmpass

        if(!newPass.equals(confirmPass)) {
            flash.passError = "Confirm Password does not match!"
        } else if(!customUserDetailsService.isValidOldPassword(oldPass, user.password)) {
            flash.passError = "Old Password does not match!"
        } else {
            user.password = newPass
            user.save flush:true

            flash.passSuccess = "Password has been updated!"
        }

        render view: 'edit', model: [user: user]
    }

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
