package ast;

import lombok.Value;

import java.util.List;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/27/16.
 */
@Value
public class InitializerList implements AssignmentExpression {
    public static final String TOKEN = ",";

    private List<AssignmentExpression> assignmentExpressions;

    @Override
    public String toCode() {
        List<String> codes = assignmentExpressions.stream().map(exp -> exp.toCode()).collect(Collectors.toList());
        return String.join(TOKEN, codes);
    }
}
