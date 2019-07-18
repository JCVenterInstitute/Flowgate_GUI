package flowgate

import groovy.transform.EqualsAndHashCode
import groovy.transform.ToString

@EqualsAndHashCode(includes='username')
@ToString(includes='username', includeNames=true, includePackage=false)
class User implements Serializable {

	private static final long serialVersionUID = 1

	transient passwordEncoder

	String username
	String password
	String email
	String affiliation
	String reason
	boolean enabled
	boolean accountExpired
	boolean accountLocked
	boolean passwordExpired

	Set<Role> getAuthorities() {
		UserRole.findAllByUser(this)*.role
	}

	def beforeInsert() {
		encodePassword()
	}

	def beforeUpdate() {
		if (isDirty('password')) {
			encodePassword()
		}
	}

	protected void encodePassword() {
		password = passwordEncoder.encode(password)
	}

	static transients = ['passwordEncoder']

	static constraints = {
		password blank: false, password: true, display: false
		username blank: false, unique: true
		email blank: true, unique: true
		affiliation nullable: false
		reason nullable: true
	}

	static mapping = {
		password column: '`password`'
	}
}
