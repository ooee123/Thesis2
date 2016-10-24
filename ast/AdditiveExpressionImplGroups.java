package ast;

import lombok.Value;
import visitor.Visitor;

import java.util.*;
import java.util.stream.Collectors;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class AdditiveExpressionImplGroups implements AdditiveExpression {
    private Collection<MultiplicativeExpression> addingTerms;
    private Collection<MultiplicativeExpression> subtractingTerms;

    @Override
    public String toCode() {
        List<String> addingTermsCode = addingTerms.stream().map(terms -> terms.toCode()).collect(Collectors.toList());
        String addingTerms = String.join(" + ", addingTermsCode);
        List<String> subtractingTermsCode = subtractingTerms.stream().map(terms -> terms.toCode()).collect(Collectors.toList());
        String subtractingTerms = String.join(" - ", subtractingTermsCode);
        if (!addingTerms.isEmpty() && !subtractingTerms.isEmpty()) {
            return addingTerms + " - " + subtractingTerms;
        } else if (addingTerms.isEmpty()) {
            return "-" + subtractingTerms;
        } else {
            return addingTerms;
        }
    }

    @Override
    public Set<String> getGuaranteedChangedVariables() {
        Set<String> lValues = new HashSet<>();
        for (MultiplicativeExpression addingTerm : addingTerms) {
            lValues.addAll(addingTerm.getGuaranteedChangedVariables());
        }
        for (MultiplicativeExpression subtractingTerm : subtractingTerms) {
            lValues.addAll(subtractingTerm.getGuaranteedChangedVariables());
        }
        return lValues;
    }

    @Override
    public Set<String> getDependentVariables() {
        Set<String> dependentVariables = new HashSet<>();
        for (MultiplicativeExpression addingTerm : addingTerms) {
            dependentVariables.addAll(addingTerm.getDependentVariables());
        }
        for (MultiplicativeExpression subtractingTerm : subtractingTerms) {
            dependentVariables.addAll(subtractingTerm.getDependentVariables());
        }
        return dependentVariables;
    }

    @Override
    public List<PostfixExpressionInvocationImpl> getInvocations() {
        List<PostfixExpressionInvocationImpl> invocations = new ArrayList<>();
        for (MultiplicativeExpression addingTerm : addingTerms) {
            invocations.addAll(addingTerm.getInvocations());
        }
        for (MultiplicativeExpression subtractingTerm : subtractingTerms) {
            invocations.addAll(subtractingTerm.getInvocations());
        }
        return invocations;
    }

    @Override
    public Set<String> getVariables() {
        Set<String> variables = new HashSet<>();
        for (MultiplicativeExpression addingTerm : addingTerms) {
            variables.addAll(addingTerm.getVariables());
        }
        for (MultiplicativeExpression subtractingTerm : subtractingTerms) {
            variables.addAll(subtractingTerm.getVariables());
        }
        return variables;
    }

    @Override
    public void visitNestedExpressions(Visitor<Void, Expression> visitor) {
        for (MultiplicativeExpression addingTerm : addingTerms) {
            visitor.visit(addingTerm);
        }
        for (MultiplicativeExpression subtractingTerm : subtractingTerms) {
            visitor.visit(subtractingTerm);
        }
    }
}
