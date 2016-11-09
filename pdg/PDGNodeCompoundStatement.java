package pdg;

import ast.*;
import com.google.common.collect.Lists;
import com.google.common.collect.Sets;
import lombok.EqualsAndHashCode;
import lombok.NonNull;
import lombok.Value;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Map;

/**
 * Created by ooee on 10/9/16.
 */
@Value
@EqualsAndHashCode(callSuper = true)
public class PDGNodeCompoundStatement extends PDGNode<CompoundStatement> {
    private Collection<PDGNode<? extends BlockItem>> body;
    private VariableMap lastAssigned;

    public PDGNodeCompoundStatement(CompoundStatement compoundStatement, Collection<PDGNode<? extends BlockItem>> body, VariableMap lastAssigned) {
        this(compoundStatement, body, lastAssigned, false);
    }

    public PDGNodeCompoundStatement(CompoundStatement compoundStatement, Collection<PDGNode<? extends BlockItem>> body, VariableMap lastAssigned, boolean required) {
        super(compoundStatement, required);
        this.body = body;
        this.lastAssigned = lastAssigned;
    }

    @Override
    public CompoundStatement sort(PDGSorter sorter) {
        Statement sort = sorter.sort(body);
        if (sort instanceof CompoundStatement) {
            return ((CompoundStatement) sort);
        } else {
            return new CompoundStatement(Lists.newArrayList(sort));
        }
    }
}
