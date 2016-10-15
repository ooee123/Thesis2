package ast;

import lombok.Value;

import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
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
    public Set<String> getLValues() {
        Set<String> lValues = new HashSet<>();
        for (MultiplicativeExpression addingTerm : addingTerms) {
            lValues.addAll(addingTerm.getLValues());
        }
        for (MultiplicativeExpression subtractingTerm : subtractingTerms) {
            lValues.addAll(subtractingTerm.getLValues());
        }
        return lValues;
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
}