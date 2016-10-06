package ast;

import lombok.Value;

import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class CommaExpression implements Expression {
    private List<AssignmentExpression> assignmentExpressions;

    @Override
    public String toCode() {
        List<String> expCodes = assignmentExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(", ", expCodes);
    }
}
