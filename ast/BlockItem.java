package ast;

import visitor.Visitor;

import java.util.Collection;
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

    void visitAllExpressions(Visitor<Expression> visitor);
}
