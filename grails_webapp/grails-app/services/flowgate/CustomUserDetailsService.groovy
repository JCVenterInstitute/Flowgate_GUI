package flowgate

import grails.plugin.springsecurity.userdetails.GormUserDetailsService
import grails.plugin.springsecurity.userdetails.GrailsUser
import grails.transaction.Transactional
import org.springframework.beans.factory.annotation.Autowired

//import org.springframework.security.core.authority.GrantedAuthorityImpl
import org.springframework.security.core.authority.SimpleGrantedAuthority
import org.springframework.security.core.userdetails.UserDetails
import org.springframework.security.core.userdetails.UsernameNotFoundException
import grails.plugin.springsecurity.SpringSecurityUtils
import org.springframework.security.crypto.password.PasswordEncoder


@Transactional
class CustomUserDetailsService extends GormUserDetailsService{

    def passwordEncoder;

    static final List NO_ROLES = [new SimpleGrantedAuthority(SpringSecurityUtils.NO_ROLE)]

    UserDetails loadUserByUsername(String username, boolean loadRoles) throws UsernameNotFoundException {
        return loadUserByUsername(username)
    }

    UserDetails loadUserByUsername(String username) throws UsernameNotFoundException {

        User.withTransaction { status ->
            User user = User.findByUsernameOrEmail(username,username)  //enable login with either username or password

            if (!user) throw new UsernameNotFoundException('User not found', username)

            def authorities = user.authorities.collect {
                new SimpleGrantedAuthority(it.authority)
            }

            return new GrailsUser(user.username,
                    user.password,
                    user.enabled,
                    !user.accountExpired,
                    !user.passwordExpired,
                    !user.accountLocked,
                    authorities ?: NO_ROLES,
                    user.id)

        }
    }

    def createUser(User user) {
        try {
            user.save flush:true
            activateUser(user, user.enabled)
        } catch (Exception e) {
            ex.printStackTrace();
        }
    }

    def activateUser(User user, boolean activate) {
        if(activate) {
            UserRole.remove user, Role.findByAuthority('ROLE_NewUser')
            UserRole.remove user, Role.findByAuthority('ROLE_Guest')
            UserRole.create user, Role.findByAuthority('ROLE_User')
        } else {
            UserRole.removeAll user
            UserRole.create user, Role.findByAuthority('ROLE_Guest')
            UserRole.create user, Role.findByAuthority('ROLE_NewUser')
        }
    }

    public boolean isValidOldPassword(String oldPassword, String password) {
        return passwordEncoder.matches(oldPassword, password);
    }
}
