package flowgate

import grails.gorm.DetachedCriteria

class ExperimentUser implements Serializable {

    Experiment experiment
    User user
    String expRole

    static ExperimentUser get(long experimentId, long userId, String erole) {
        criteriaFor(experimentId, userId, erole).get()
    }

    static boolean exists(long experimentId, long userId, String erole) {
        criteriaFor(experimentId, userId, erole).count()
    }

    private static DetachedCriteria criteriaFor(long experimentId, long userId, String erole) {
        ExperimentUser.where {
            experiment == Experiment.load(experimentId) &&
                user == User.load(userId) &&
                    expRole == erole
        }
    }

    static ExperimentUser create(Experiment experiment, User user, String erole) {
        def instance = new ExperimentUser(experiment: experiment, user: user, expRole:  erole)
        instance.save()
        instance
    }

    static boolean remove(Experiment e, User u, String r) {
        if (e != null && u != null && r != '') {
            ExperimentUser.where {experiment == e && user == u && expRole == r }.deleteAll()
        }
    }

    static int removeAll(User u) {
        u == null ? 0 : ExperimentUser.where { user == u }.deleteAll()
    }

    static int removeAll(Experiment e) {
        e == null ? 0 : ExperimentUser.where { experiment == e }.deleteAll()
    }

    static constraints = {
        experiment validator: { Experiment e, ExperimentUser eu ->
            if (eu.user?.id) {
                ExperimentUser.withNewSession {
                    if (ExperimentUser.exists(e.id, eu.user.id, eu.expRole)) {
                        return ['experimentUserRole.exists']
                    }
                }
            }
        }
    }

    static mapping = {
        id composite: ['experiment', 'user', 'expRole']
//        version false
    }
}
