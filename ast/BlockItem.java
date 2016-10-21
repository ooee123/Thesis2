package ast;

import java.util.HashSet;
import java.util.Set;

/**
 * Created by ooee on 9/24/16.
 */
public interface BlockItem extends BaseElement {
    boolean isCritical();

    Set<String> getDependantVariables();

    default Set<String> getChangedVariables() {
        Set<String> changedVariables = new HashSet<>();
        changedVariables.addAll(getPotentiallyChangedVariables());
        changedVariables.addAll(getGuaranteedChangedVariables());
        return changedVariables;
    }

    Set<String> getGuaranteedChangedVariables();

    Set<String> getPotentiallyChangedVariables();
    /*
    default Set<String> getPotentiallyChangedVariables() {
        Set<String> potentiallyChangedVariables = new HashSet<>(getChangedVariables());
        potentiallyChangedVariables.removeAll(getGuaranteedChangedVariables());
        return potentiallyChangedVariables;
    }
    */
}
