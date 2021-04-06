package flowgate.transform

class PolygonGate extends Gate {

    def markerNames = []
    def markerLabels = []
    def xCoordinates = []
    def yCoordinates = []
    def transformationRefs = []

    @Override
    public String toString() {
        return "PolygonGate{" +
                "xCoordinates=" + xCoordinates +
                ", yCoordinates=" + yCoordinates +
                ", transformationRefs=" + transformationRefs +
                '}';
    }
}
