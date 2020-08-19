package flowgate

import grails.gorm.DetachedCriteria

class AnalysisDataset implements Serializable {

    private static final long serialVersionUID = 1

    Analysis analysis
    Dataset dataset
    Integer dsVersion

    static AnalysisDataset get(long analysisId, long datasetId) {
        criteriaFor(analysisId, datasetId).get()
    }

    static boolean exists(long analysisId, long datasetId) {
        criteriaFor(analysisId, datasetId).count()
    }

    private static DetachedCriteria criteriaFor(long analysisId, long datasetId) {
        AnalysisDataset.where {
            analysis == Analysis.load(analysisId) && dataset == Dataset.load(datasetId)
        }
    }

    static AnalysisDataset create(Analysis analysis, Dataset dataset) {
        def instance = new AnalysisDataset(analysis: analysis, dataset: dataset, dsVersion: dataset.version)
        instance.save( flush: true)
        instance
    }

    static boolean remove(Analysis a, Dataset d) {
        if (a != null && d != null) {
            AnalysisDataset.where { analysis == a && dataset == d }.deleteAll()
        }
    }

    static int removeAll(Analysis a) {
        a == null ? 0 : AnalysisDataset.where { analysis == a }.deleteAll()
    }

    static int removeAll(Dataset d) {
        d == null ? 0 : AnalysisDataset.where { dataset == d }.deleteAll()
    }

    static constraints = {
    }

    static mapping = {
        id composite: ['analysis', 'dataset']
        version false
    }
}


