package ast;

import lombok.Value;

import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/27/16.
 */
@Value
public class InitializerList implements AssignmentExpression, Assigning {
    public static final String TOKEN = ",";

    private List<AssignmentExpression> assignmentExpressions;

    @Override
    public String toCode() {
        List<String> codes = assignmentExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(TOKEN, codes);
    }

    @Override
    public Set<String> getLValues() {
        Set<String> lValues = new HashSet<>();
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            lValues.addAll(assignmentExpression.getLValues());
        }
        return lValues;
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            variables.addAll(assignmentExpression.getVariables());
        }
        return variables;
    }

    @Override
    public Set<String> getRightVariables() {
        Set<String> rightVariables = new HashSet<>();
        for (AssignmentExpression assignmentExpression : assignmentExpressions) {
            if (assignmentExpression instanceof Assigning) {
                rightVariables.addAll(((Assigning) assignmentExpression).getRightVariables());
            }
        }
        return rightVariables;
    }
}
