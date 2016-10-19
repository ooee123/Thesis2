package ast;

import pdg.PDGNode;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/24/16.
 */
public interface Statement extends BlockItem {
    Set<String> getDependantVariables();

    Set<String> getChangedVariables();

    Set<String> getGuaranteedChangedVariables();

    <T extends PDGNode> T getPDGNode();

    default Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>(getChangedVariables());
        potentiallyChangedVariables.removeAll(getGuaranteedChangedVariables());
        return potentiallyChangedVariables;
    }
}
