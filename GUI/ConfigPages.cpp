
#include <QtWidgets>
#include <QVector>
#include <boost/foreach.hpp>

#include "ConfigPages.h"

MyLineEdit::MyLineEdit(QWidget *parent)
    : QLineEdit(parent)
{
    this->setFocusPolicy(Qt::StrongFocus);
}

MyLineEdit::~MyLineEdit()
{}

void MyLineEdit::focusInEvent(QFocusEvent *e)
{
//    std::cout << "focussedIn!" << std::endl;
    emit(focussed(true));
    QLineEdit::focusInEvent(e);
}

void MyLineEdit::focusOutEvent(QFocusEvent *e)
{
//    std::cout << "focussedOut!" << std::endl;
    QLineEdit::focusOutEvent(e);
//    emit(focussed(false));
}

void
displayHelp(QString f, QTextEdit* help_box)
{
    if (!QFile::exists(f)) return;
    QFile file(f);
    if (!file.open(QFile::ReadOnly)) return;
    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        help_box->setHtml(str);
    } else {
        str = QString::fromLocal8Bit(data);
        help_box->setPlainText(str);
    }
}

ProblemSpecPage::ProblemSpecPage(ConfigDialog * config_dialogue, QTextEdit * _help_box, QWidget *parent)
    : QWidget(parent),
      objectives_List(new QListWidget),
//      years_List(new QListWidget),
      xpath_List(new QListWidget),
//      rate_edit(new QLineEdit),
//      discount_year_zero_edit(new QLineEdit),
//      year_begin_metrics_edit(new QLineEdit),
//      year_end_metrics_edit(new QLineEdit),
      zone_delineation_edit(new MyLineEdit(this)),
      zonal_layer_edit(new MyLineEdit(this)),
      evaluator_modules_list(new QListWidget),
        zonal_map_classes_edit(new MyLineEdit(this)),
      help_box(_help_box)
{
    QGroupBox *objectives_Group = new QGroupBox(tr("Objectives"));

    QLabel *objectives_Label = new QLabel(tr("Maps to aggregate to get objective values:"));
//    QListWidget *objectives_List = new QListWidget;
    QPushButton* add_obj_button = new QPushButton(tr("Add new map"));
//    QLabel *years_Label = new QLabel(tr("Years to aggregate to get objective values:"));
//    QListWidget *objectives_List = new QListWidget;
//    QPushButton* add_year_button = new QPushButton(tr("Add new year"));
//    QLabel *rate_label = new QLabel(tr("Discount rate:"));
//    QLabel *year_zero_label = new QLabel(tr("Year calculate present value at:"));
    
    QLabel *evaluator_modules_Label = new QLabel(tr("Evaluator Modules for other objective values:"));
//    QListWidget *objectives_List = new QListWidget;
    QPushButton* evaluator_modules_button = new QPushButton(tr("Add new evaluator module"));
    
//    QLabel *year_begin_label = new QLabel(tr("Start calculating map aggregation in year:"));
//    QLabel *year_end_label = new QLabel(tr("End calculating map aggregation in year:"));
    
//    QLineEdit *rate_edit = new QLineEdit;

    QGridLayout *objectives_Layout = new QGridLayout;
    objectives_Layout->addWidget(objectives_Label,1,0,1,2);
    objectives_Layout->addWidget(objectives_List,2,0,1,2);
    objectives_Layout->addWidget(add_obj_button,3,0,1,2);
//    objectives_Layout->addWidget(years_Label,4,0,1,2);
//    objectives_Layout->addWidget(years_List,5,0,1,2);
//    objectives_Layout->addWidget(add_year_button,6,0,1,2);
//    objectives_Layout->addWidget(year_begin_label, 4, 0);
//    objectives_Layout->addWidget(year_begin_metrics_edit, 4, 1);
//    objectives_Layout->addWidget(year_end_label, 5, 0);
//    objectives_Layout->addWidget(year_end_metrics_edit, 5, 1);
    objectives_Layout->addWidget(evaluator_modules_Label,7,0,1,2);
    objectives_Layout->addWidget(evaluator_modules_list,8,0,1,2);
    objectives_Layout->addWidget(evaluator_modules_button,9,0,1,2);
//    objectives_Layout->addWidget(rate_label,10,0);
//    objectives_Layout->addWidget(rate_edit,10,1);
//    objectives_Layout->addWidget(year_zero_label,11,0);
//    objectives_Layout->addWidget(discount_year_zero_edit,11,1);

    objectives_Group->setLayout(objectives_Layout);


    connect(objectives_List, &QListWidget::itemChanged, this, &ProblemSpecPage::processObjectiveMapListChange);
    connect(this, &ProblemSpecPage::objectiveMapsChanged, config_dialogue, &ConfigDialog::changeObjectiveMaps);
    connect(add_obj_button, &QPushButton::clicked, this, &ProblemSpecPage::addObjectiveMap);

//    connect(years_List, &QListWidget::itemChanged, this, &ProblemSpecPage::processYearListChange);
//    connect(this, &ProblemSpecPage::yearsCalculatedChanged, config_dialogue, &ConfigDialog::changeMetricYears);
//    connect(add_obj_button, &QPushButton::clicked, this, &ProblemSpecPage::addYear);

//    connect(rate_edit, &QLineEdit::textEdited, config_dialogue, &ConfigDialog::changeDiscountRate);
//    connect(discount_year_zero_edit, &QLineEdit::textEdited, config_dialogue, &ConfigDialog::changeDiscountRate);
    
//    connect(year_begin_metrics_edit, &QLineEdit::textEdited, config_dialogue, &ConfigDialog::changeYearStartMetrics);
//    connect(year_end_metrics_edit, &QLineEdit::textEdited, config_dialogue, &ConfigDialog::changeYearEndMetrics);
    
    connect(evaluator_modules_list, &QListWidget::itemChanged, this, &ProblemSpecPage::processObjModuleChange);
    connect(this, &ProblemSpecPage::objModulesChanged, config_dialogue, &ConfigDialog::changeObjModules);
    connect(evaluator_modules_button, &QPushButton::clicked, this, &ProblemSpecPage::addObjModule);

    connect(objectives_List, &QListWidget::itemClicked, this, &ProblemSpecPage::displayObjMapHelp);
    connect(evaluator_modules_list, &QListWidget::itemClicked, this, &ProblemSpecPage::displayObjModuleHelp);
    connect(xpath_List, &QListWidget::itemClicked, this, &ProblemSpecPage::displayXPathDVsHelp);


//    connect(objectives_Label, &QLabel::)

    ///////

    QGroupBox *zonal_Group = new QGroupBox(tr("Zonal optimisation"));

    QLabel *zone_delineation_label = new QLabel(tr("Map specifying zones"));
//    QLineEdit *zone_delineation_edit = new QLineEdit;
    QLabel *zonal_layer_label = new QLabel(tr("Zonal layer in Geonamica"));
//    QLineEdit *zonal_layer_edit = new QLineEdit;
    QLabel *zonal_map_classes_label = new QLabel(tr("Zonal classes as defined in zonal tool in geopoproject/Geonamica"));

    QVBoxLayout *zonal_Layout = new QVBoxLayout;
    zonal_Layout->addWidget(zone_delineation_label);
    zonal_Layout->addWidget(zone_delineation_edit);
    zonal_Layout->addSpacing(12);
    zonal_Layout->addWidget(zonal_layer_label);
    zonal_Layout->addWidget(zonal_layer_edit);
    zonal_Layout->addSpacing(12);
    zonal_Layout->addWidget(zonal_map_classes_label);
    zonal_Layout->addWidget(zonal_map_classes_edit);

    zonal_Group->setLayout(zonal_Layout);

    connect(zone_delineation_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeZoneDelineation);
    connect(zonal_layer_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeZonalLayer);
     connect(zonal_map_classes_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeZonalClasses);

    connect(zone_delineation_edit, &MyLineEdit::focussed, this, &ProblemSpecPage::displayZonalOptimisationHelp);
    connect(zonal_layer_edit, &MyLineEdit::focussed, this, &ProblemSpecPage::displayZonalOptimisationHelp);
    connect(zonal_map_classes_edit, &MyLineEdit::focussed, this, &ProblemSpecPage::displayZonalOptimisationHelp);

    ///////

    QGroupBox *xpath_Group = new QGroupBox(tr("Xpath decision variables"));
    QLabel *xpath_Label = new QLabel(tr("Xpaths pointing to decision variables in geoproject:"));
//    QListWidget *xpath_List = new QListWidget;
    QPushButton* add_xpath_button = new QPushButton(tr("Add new xpath"));

    QVBoxLayout *xpath_Layout = new QVBoxLayout;
    xpath_Layout->addWidget(xpath_Label);
    xpath_Layout->addWidget(xpath_List);
    xpath_Layout->addWidget(add_xpath_button);

    xpath_Group->setLayout(xpath_Layout);

    connect(xpath_List, &QListWidget::itemChanged, this, &ProblemSpecPage::processOXPathDVListChange);
    connect(this, &ProblemSpecPage::xpathDVsChanged, config_dialogue, &ConfigDialog::changeXPathDVs);
    connect(add_xpath_button, &QPushButton::clicked, this, &ProblemSpecPage::addXPathDV);

    //////

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(objectives_Group);
    mainLayout->addWidget(zonal_Group);
    mainLayout->addWidget(xpath_Group);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void ProblemSpecPage::addObjectiveMap()
{
    QListWidgetItem * new_item = new QListWidgetItem(objectives_List);
    new_item->setText(tr("New objective map"));
    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
//    objectives_List->insert(new_item);
}

void ProblemSpecPage::addYear()
{
    QListWidgetItem * new_item = new QListWidgetItem(years_List);
    new_item->setText(tr("New year to include in objective calculations"));
    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
    //    objectives_List->insert(new_item);
}

void ProblemSpecPage::addXPathDV()
{
    QListWidgetItem * new_item = new QListWidgetItem(xpath_List);
    new_item->setText(tr("New XPath decision variable"));
    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
}

void ProblemSpecPage::processObjectiveMapListChange()
{
    QVector<QString> items;
    for (int j = 0; j < objectives_List->count() ; ++j)
    {
        items.append(objectives_List->item(j)->text());
    }
    emit objectiveMapsChanged(items);
}

//void ProblemSpecPage::processYearListChange()
//{
//    QVector<QString> items;
//    for (int j = 0; j < years_List->count() ; ++j)
//    {
//        items.append(years_List->item(j)->text());
//    }
//    emit objectiveMapsChanged(items);
//}

void ProblemSpecPage::processOXPathDVListChange()
{
    QVector<QString> items;
    for (int j = 0; j < xpath_List->count() ; ++j)
    {
        items.append(xpath_List->item(j)->text());
    }
    emit xpathDVsChanged(items);
}

void ProblemSpecPage::updateObjectiveMaps(std::vector<std::string> obj_maps)
{
    BOOST_FOREACH(std::string & obj_map, obj_maps)
                {
                    QListWidgetItem * new_item = new QListWidgetItem(objectives_List);
                    new_item->setText(QString::fromStdString(obj_map));
                    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
                }
//    std::for_each(obj_maps.begin(), obj_maps.end(), [*this](std::string& obj_map) {objectives_List->addItem(QString::fromStdString(obj_map));});
}

//void ProblemSpecPage::updateDiscountRate(double rate)
//{
//    rate_edit->setText(QString::number(rate));
//}

void ProblemSpecPage::updateZoneDelineationMap(QString path)
{
    zone_delineation_edit->setText(path);
}

void ProblemSpecPage::updateZonalMapClasses(QString values)
{
    zonal_map_classes_edit->setText(values);
}

void ProblemSpecPage::updateZonalLayerMap(QString path)
{
    zonal_layer_edit->setText(path);
}

void ProblemSpecPage::updateXpathDVs(std::vector<std::string> xpathDVs)
{
    BOOST_FOREACH(std::string & xpath_dv, xpathDVs)
                {
                    QListWidgetItem * new_item = new QListWidgetItem(xpath_List);
                    new_item->setText(QString::fromStdString(xpath_dv));
                    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
                }
//    std::for_each(xpathDVs.begin(), xpathDVs.end(), [xpath_List](std::string& xpath_dv) {xpath_List->addItem(QString::fromStdString(xpath_dv));});
//    QVectorIterator<QString> i(xpathDVs);
//    while(i.hasNext())
//    {
//        xpath_List->addItem(i.next());
//    }
}

void ProblemSpecPage::updateObjModules(std::vector<std::string> obj_modules)
{
    BOOST_FOREACH(std::string & obj_module, obj_modules)
                {
                    QListWidgetItem * new_item = new QListWidgetItem(evaluator_modules_list);
                    new_item->setText(QString::fromStdString(obj_module));
                    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
                }
}

//void ProblemSpecPage::updateStartYear(int year)
//{
//    year_begin_metrics_edit->setText(QString::number(year));
//}
//
//void ProblemSpecPage::updateEndYear(int year)
//{
//    year_end_metrics_edit->setText(QString::number(year));
//}

void ProblemSpecPage::addObjModule()
{
    QListWidgetItem * new_item = new QListWidgetItem(evaluator_modules_list);
    new_item->setText(tr("New objective module specification"));
    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
}

void ProblemSpecPage::processObjModuleChange()
{
    QVector<QString> items;
    for (int j = 0; j < evaluator_modules_list->count() ; ++j)
    {
        items.append(evaluator_modules_list->item(j)->text());
    }
    emit objModulesChanged(items);
}

void ProblemSpecPage::displayObjMapHelp()
{

    QString f = ":/Help/ObjMap.html";
    displayHelp(f, help_box);
}

void ProblemSpecPage::displayObjModuleHelp()
{
    QString f = ":/Help/EvalModules.html";
    displayHelp(f, help_box);
}

void ProblemSpecPage::displayXPathDVsHelp()
{
    QString f = ":/Help/XPathDVs.html";
    displayHelp(f, help_box);
}

void ProblemSpecPage::displayZonalOptimisationHelp()
{
    QString f = ":/Help/ZonalOptimisation.html";
    displayHelp(f, help_box);
}

GeonSettingsPage::GeonSettingsPage(ConfigDialog* config_dialogue, QTextEdit * _help_box, QWidget *parent)
    : QWidget(parent),
      save_map_List(new QListWidget),
      timeout_edit(new MyLineEdit),
      wine_edit(new MyLineEdit),
      geon_edit(new MyLineEdit),
      working_dir_edit(new MyLineEdit),
      wine_prefix_edit(new MyLineEdit),
      saving_dir_edit(new MyLineEdit),
      testing_dir_edit(new MyLineEdit),
      log_checkbox(new QCheckBox(tr("Log the optimisation (i.e. for debugging)"))),
      prefix_env_var_CheckBox(new QCheckBox(tr("Set wine prefix environment variable on system call"))),
      windows_env_var_edit(new MyLineEdit),
      geoproj_directory_edit(new MyLineEdit),
      geoproj_file_edit(new MyLineEdit),
      obj_log_file_edit(new MyLineEdit),
      plot_log_file_edit(new MyLineEdit),
//      year_begin_save_edit(new QLineEdit),
//      year_end_save_edit(new QLineEdit),
      replicates_SpinBox(new QSpinBox),
      help_box(_help_box),
      driver_letter_edit(new MyLineEdit),
      do_throw_exceptns_CheckBox(new QCheckBox(tr("Throw exceptions halting optimisation")))
{
    QGroupBox* system_call_group = new QGroupBox(tr("System commands to run Geonamica"));

    QLabel *timeout_label = new QLabel(tr("Timeout command:"));
//    QLineEdit *timeout_edit = new QLineEdit;
    QLabel *wine_label = new QLabel(tr("Wine command:"));
//    QLineEdit *wine_edit = new QLineEdit;
    QLabel *geon_label = new QLabel(tr("Geonamica command:"));
//    QLineEdit *geon_edit = new QLineEdit;
    QLabel *windows_env_var_label = new QLabel(tr("Windows environment variable to set:"));


    QGridLayout *system_calls_Layout = new QGridLayout;
    system_calls_Layout->addWidget(timeout_label, 0, 0);
    system_calls_Layout->addWidget(timeout_edit, 0, 1);
    system_calls_Layout->addWidget(wine_label, 1, 0);
    system_calls_Layout->addWidget(wine_edit, 1, 1);
    system_calls_Layout->addWidget(geon_label, 2, 0);
    system_calls_Layout->addWidget(geon_edit, 2, 1);

    system_call_group->setLayout(system_calls_Layout);


    connect(timeout_edit, &QLineEdit::textEdited, config_dialogue, &ConfigDialog::changeTimeoutCmd);
    connect(wine_edit, &QLineEdit::textEdited, config_dialogue, &ConfigDialog::changeWineCmd);
    connect(geon_edit, &QLineEdit::textEdited, config_dialogue, &ConfigDialog::changeGeonCmd);

    connect(timeout_edit, &MyLineEdit::focussed, this, &GeonSettingsPage::displayTimeoutHelp);
    connect(wine_edit, &MyLineEdit::focussed, this, &GeonSettingsPage::displayWineCmdHelp);
    connect(geon_edit, &MyLineEdit::focussed, this, &GeonSettingsPage::displayGeonamicaCmdHelp);

    //////

    QGroupBox* geonamica_config_group = new QGroupBox(tr("Geonamica runtime environment"));

    QLabel *working_dir_label = new QLabel(tr("Working directory:"));
//    QLineEdit *working_dir_edit = new QLineEdit;
    QLabel *wine_prefix_label = new QLabel(tr("Wine prefix path:"));
//    QLineEdit *wine_prefix_edit = new QLineEdit;
//    QCheckBox* prefix_env_var_CheckBox = new QCheckBox(tr("Set wine prefix environment variable on system call"));
    QLabel *saving_dir_label = new QLabel(tr("Saving directory:"));
    QLabel *testing_dir_label = new QLabel(tr("Testing directory:"));
//    QLineEdit *saving_dir_edit = new QLineEdit;
//    QCheckBox* log_checkbox = new QCheckBox(tr("Log the optimisation (i.e. for debugging)"));
    QLabel* drive_letter_label = new QLabel(tr("Drive letter to set for Georproject directory"));

    QGridLayout *geon_env_Layout = new QGridLayout;
    geon_env_Layout->addWidget(working_dir_label, 0, 0);
    geon_env_Layout->addWidget(working_dir_edit, 0, 1);
    geon_env_Layout->addWidget(wine_prefix_label, 1, 0);
    geon_env_Layout->addWidget(wine_prefix_edit, 1, 1);
    geon_env_Layout->addWidget(prefix_env_var_CheckBox, 2, 0, 1, 2);
    geon_env_Layout->addWidget(drive_letter_label, 3,0);
    geon_env_Layout->addWidget(driver_letter_edit, 3,1);
    geon_env_Layout->addWidget(windows_env_var_label, 4, 0);
    geon_env_Layout->addWidget(windows_env_var_edit, 4, 1);
    geon_env_Layout->addWidget(saving_dir_label, 5, 0);
    geon_env_Layout->addWidget(saving_dir_edit, 5, 1);
    geon_env_Layout->addWidget(testing_dir_label, 6, 0);
    geon_env_Layout->addWidget(testing_dir_edit, 6, 1);
    geon_env_Layout->addWidget(log_checkbox, 7, 0);
    geon_env_Layout->addWidget(do_throw_exceptns_CheckBox, 8, 0);


    geonamica_config_group->setLayout(geon_env_Layout);

    connect(working_dir_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeWorkingDirectory);
    connect(wine_prefix_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changePrefixPath);
    connect(driver_letter_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeDriveLetter);
    connect(saving_dir_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeSaveDir);
    connect(testing_dir_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeTestDir);
    connect(windows_env_var_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeWindowsEnvVar);
    connect(prefix_env_var_CheckBox, &QCheckBox::stateChanged, config_dialogue, &ConfigDialog::changeWhetherPrefixEnvVarSet);
    connect(log_checkbox, &QCheckBox::stateChanged, config_dialogue, &ConfigDialog::changeDoLog);
    connect(do_throw_exceptns_CheckBox, &QCheckBox::stateChanged, config_dialogue, &ConfigDialog::changeDoThrowExceptns);

    connect(working_dir_edit, &MyLineEdit::focussed, this, &GeonSettingsPage::displayWorkDirHelp);
    connect(wine_prefix_edit, &MyLineEdit::focussed, this, &GeonSettingsPage::displayWinePrefixHelp);
    connect(driver_letter_edit, &MyLineEdit::focussed, this, &GeonSettingsPage::displayDriverLetterHelp);

    //////

    QGroupBox* geonamica_files_group = new QGroupBox(tr("Geoproject and logging files"));

    QLabel *geoproj_directory_label = new QLabel(tr("Geoproject directory:"));
//    QLineEdit *geoproj_directory_edit = new QLineEdit;
    QLabel *geoproj_file_label = new QLabel(tr("Geoproject file:"));
//    QLineEdit *geoproj_file_edit = new QLineEdit;
    QLabel *obj_log_file_label = new QLabel(tr("Log file for objectives:"));
//    QLineEdit *obj_log_file_edit = new QLineEdit;
    QLabel *plot_log_file_label = new QLabel(tr("Log file for post-optimisation plotting of results:"));
//    QLineEdit *plot_log_file_edit = new QLineEdit;

    QGridLayout *geon_files_Layout = new QGridLayout;
    geon_files_Layout->addWidget(geoproj_directory_label, 0, 0);
    geon_files_Layout->addWidget(geoproj_directory_edit, 0, 1);
    geon_files_Layout->addWidget(geoproj_file_label, 1, 0);
    geon_files_Layout->addWidget(geoproj_file_edit, 1, 1);
    geon_files_Layout->addWidget(obj_log_file_label, 2, 0);
    geon_files_Layout->addWidget(obj_log_file_edit, 2, 1);
    geon_files_Layout->addWidget(plot_log_file_label, 3, 0);
    geon_files_Layout->addWidget(plot_log_file_edit, 3, 1);

    geonamica_files_group->setLayout(geon_files_Layout);

    connect(geoproj_directory_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeGeoprojDirectory);
    connect(geoproj_file_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeGeoprojectFile);
    connect(obj_log_file_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeLogFileForObjective);
    connect(plot_log_file_edit, &MyLineEdit::textEdited, config_dialogue, &ConfigDialog::changeLogFileForPostOptimisationSave);


    //////

    QGroupBox* save_files_group = new QGroupBox(tr("Maps to save post-optimisation from Pareto front"));

    QLabel *save_map_Label = new QLabel(tr("Paths pointing to output log maps"));
//    QListWidget *save_map_List = new QListWidget;
    QPushButton* add_save_map_button = new QPushButton(tr("Add new map"));
//    QLabel *year_begin_label = new QLabel(tr("Start saving maps in year:"));
//    QLineEdit *year_begin_edit = new QLineEdit;
//    QLabel *year_end_label = new QLabel(tr("End saving maps in year:"));
//    QLineEdit *year_end_edit = new QLineEdit;

    QGridLayout *save_files_Layout = new QGridLayout;
    save_files_Layout->addWidget(save_map_Label, 0, 0, 1, 2);
    save_files_Layout->addWidget(save_map_List, 1, 0 , 1, 2);
    save_files_Layout->addWidget(add_save_map_button, 2, 0, 1, 2);
//    save_files_Layout->addWidget(year_begin_label, 3, 0);
//    save_files_Layout->addWidget(year_begin_save_edit, 3, 1);
//    save_files_Layout->addWidget(year_end_label, 4, 0);
//    save_files_Layout->addWidget(year_end_save_edit, 4, 1);



    save_files_group->setLayout(save_files_Layout);

    connect(save_map_List, &QListWidget::itemChanged, this, &GeonSettingsPage::processOutputLogMapListChange);
    connect(this, &GeonSettingsPage::outputLogMapsChanged, config_dialogue, &ConfigDialog::changeSaveMaps);
    connect(add_save_map_button, &QPushButton::clicked, this, &GeonSettingsPage::addOutputLogMap);
    connect(save_map_List, &QListWidget::itemClicked, this, &GeonSettingsPage::displaySaveMapHelp);
//    connect(year_begin_save_edit, &QLineEdit::textEdited, config_dialogue, &ConfigDialog::changeYearStartSave);
//    connect(year_end_save_edit, &QLineEdit::textEdited, config_dialogue, &ConfigDialog::changeYearEndSave);

    /////

    QGroupBox* replications_group = new QGroupBox(tr("Replication accouting for model stochasticity"));

//    QSpinBox *replicates_SpinBox = new QSpinBox;
    replicates_SpinBox->setPrefix(tr("Run Geonamica "));
    replicates_SpinBox->setSuffix(tr(" times for each evaluation of the objectives"));
    replicates_SpinBox->setSpecialValueText(tr("Run Geonamica only once"));
    replicates_SpinBox->setMinimum(1);
    replicates_SpinBox->setMaximum(100);
    replicates_SpinBox->setSingleStep(1);

    QVBoxLayout *replicate_Layout = new QVBoxLayout;
    replicate_Layout->addWidget(replicates_SpinBox);

    replications_group->setLayout(replicate_Layout);

    connect(replicates_SpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), config_dialogue, &ConfigDialog::changeNumberReplicates);

    ////


    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(system_call_group);
    mainLayout->addWidget(geonamica_config_group);
    mainLayout->addWidget(geonamica_files_group);
    mainLayout->addWidget(save_files_group);
    mainLayout->addWidget(replications_group);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void GeonSettingsPage::displaySaveMapHelp()
{
    QString f = ":/Help/SaveMaps.html";
    displayHelp(f, help_box);
}

void GeonSettingsPage::addOutputLogMap()
{
    QListWidgetItem * new_item = new QListWidgetItem(save_map_List);
    new_item->setText(tr("New output log map"));
    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);

}

void GeonSettingsPage::processOutputLogMapListChange()
{
    QVector<QString> items;
    for (int j = 0; j < save_map_List->count() ; ++j)
    {
        items.append(save_map_List->item(j)->text());
    }
    emit outputLogMapsChanged(items);

}

void GeonSettingsPage::updateTimeoutCmd(QString path)
{
    timeout_edit->setText(path);
}

void GeonSettingsPage::updateWineCmd(QString path)
{
    wine_edit->setText(path);
}

void GeonSettingsPage::updateGeonCmd(QString path)
{
    geon_edit->setText(path);
}

void GeonSettingsPage::updateworkingDir(QString path)
{
    working_dir_edit->setText(path);
}

void GeonSettingsPage::updateWinePrefix(QString path)
{
    wine_prefix_edit->setText(path);
}

void GeonSettingsPage::updateSaveDir(QString path)
{
    saving_dir_edit->setText(path);
}

void GeonSettingsPage::updateWhetherPrefixEnvVarSet(bool do_set)
{
    if (do_set) prefix_env_var_CheckBox->setCheckState(Qt::Checked);
    else prefix_env_var_CheckBox->setCheckState(Qt::Unchecked);

}

void GeonSettingsPage::updateWindowsEnvVar(QString val)
{
    windows_env_var_edit->setText(val);
}

void GeonSettingsPage::updateWhetherLog(bool do_log)
{
    if (do_log) log_checkbox->setCheckState(Qt::Checked);
    else log_checkbox->setCheckState(Qt::Unchecked);
}

void GeonSettingsPage::updateGeoprojDir(QString path)
{
    geoproj_directory_edit->setText(path);
}

void GeonSettingsPage::updateGeoprojFile(QString path)
{
    geoproj_file_edit->setText(path);
}

void GeonSettingsPage::updateObjLogFile(QString path)
{
    obj_log_file_edit->setText(path);
}

void GeonSettingsPage::updatePostOptPrintLogFile(QString path)
{
    plot_log_file_edit->setText(path);
}

//void GeonSettingsPage::updateStartYear(int year)
//{
//    year_begin_save_edit->setText(QString::number(year));
//}
//
//void GeonSettingsPage::updateEndYear(int year)
//{
//    year_end_save_edit->setText(QString::number(year));
//}

void GeonSettingsPage::updateNumberReplicates(int number)
{
    replicates_SpinBox->setValue(number);
}

void GeonSettingsPage::updateOuputLogMaps(std::vector<std::string> ouput_maps)
{
    BOOST_FOREACH(std::string & ouput_map, ouput_maps)
                {
                    QListWidgetItem * new_item = new QListWidgetItem(save_map_List);
                    new_item->setText(QString::fromStdString(ouput_map));
                    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
                }
//    std::for_each(ouput_maps.begin(), ouput_maps.end(), [save_map_List](std::string& xpath_dv) {save_map_List->addItem(QString::fromStdString(xpath_dv));});
}

void GeonSettingsPage::updateDriveLetter(QString letter)
{
    driver_letter_edit->setText(letter);
}
void
GeonSettingsPage::updateTestDir(QString path)
{
    testing_dir_edit->setText(path);
}

void GeonSettingsPage::displayTimeoutHelp()
{
    QString f = ":/Help/timeout.html";
    displayHelp(f, help_box);
}

void GeonSettingsPage::displayWineCmdHelp() {
    QString f = ":/Help/WineCmd.html";
    displayHelp(f, help_box);
}

void GeonSettingsPage::displayGeonamicaCmdHelp() {
    QString f = ":/Help/GeonamicaCommand.html";
    displayHelp(f, help_box);
}

void GeonSettingsPage::displayWorkDirHelp() {
    QString f = ":/Help/Workingdirectory.html";
    displayHelp(f, help_box);
}

void GeonSettingsPage::displayWinePrefixHelp() {
    QString f = ":/Help/WinePrefixPath.html";
    displayHelp(f, help_box);
}

void GeonSettingsPage::displayDriverLetterHelp() {
    QString f = ":/Help/WindDriveLetter.html";
    displayHelp(f, help_box);
}
void
GeonSettingsPage::updateWhetherThrowExceptns(bool do_throw)
{
    if (do_throw) do_throw_exceptns_CheckBox->setCheckState(Qt::Checked);
    else do_throw_exceptns_CheckBox->setCheckState(Qt::Unchecked);
}

EAPage::EAPage(ConfigDialog* config_dialogue, QTextEdit * _help_box, QWidget *parent)
    : QWidget(parent),
      pop_SpinBox(new QSpinBox),
        hyprvol_term_SpinBox(new QSpinBox),
        term_SpinBox(new QSpinBox),
        reseed_edit(new QLineEdit),
        logging_SpinBox(new QSpinBox),
      email_address_List(new QListWidget),
      help_box(_help_box)
{
    QGroupBox *parameters_Group = new QGroupBox(tr("NSGAII parameters"));

//    QSpinBox *pop_SpinBox = new QSpinBox;
    pop_SpinBox->setPrefix(tr("Run GA with population size of "));
    pop_SpinBox->setMinimum(10);
    pop_SpinBox->setMaximum(1000);
    pop_SpinBox->setSingleStep(50);

    QVBoxLayout* parameters_layout = new QVBoxLayout;
    parameters_layout->addWidget(pop_SpinBox);

    parameters_Group->setLayout(parameters_layout);

    connect(pop_SpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), config_dialogue, &ConfigDialog::changePopSize);

    /////

    QGroupBox *termination_Group = new QGroupBox(tr("Termination conditions"));

//    QSpinBox *hyprvol_term_SpinBox = new QSpinBox;
    hyprvol_term_SpinBox->setPrefix(tr("Terminate GA after "));
    hyprvol_term_SpinBox->setSuffix(tr(" generations with no hypervolume improvement"));
    hyprvol_term_SpinBox->setMinimum(10);
    hyprvol_term_SpinBox->setMaximum(200);
    hyprvol_term_SpinBox->setSingleStep(10);

//    QSpinBox *term_SpinBox = new QSpinBox;
    term_SpinBox->setPrefix(tr("Terminate GA after "));
    term_SpinBox->setSuffix(tr(" generations"));
    term_SpinBox->setMinimum(1);
    term_SpinBox->setMaximum(10000);
    term_SpinBox->setSingleStep(100);

    QVBoxLayout * termination_layout = new QVBoxLayout;
    termination_layout->addWidget(hyprvol_term_SpinBox);
    termination_layout->addWidget(term_SpinBox);

    termination_Group->setLayout(termination_layout);

    connect(hyprvol_term_SpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), config_dialogue, &ConfigDialog::changeMaxGenHypImprove);
    connect(term_SpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), config_dialogue, &ConfigDialog::changeMaxGen);

    ///////

    QGroupBox *reseed = new QGroupBox(tr("Reseeding"));

    QLabel* ressed_label = new QLabel(tr("Reseed population file:"));
//    QLineEdit *reseed_edit = new QLineEdit;

    QGridLayout* reseed_layout = new QGridLayout;
    reseed_layout->addWidget(ressed_label, 0 , 0);
    reseed_layout->addWidget(reseed_edit, 0 , 1);

    reseed->setLayout(reseed_layout);

    connect(reseed_edit, &QLineEdit::textEdited, config_dialogue, &ConfigDialog::changeRessed);

    ////

    QGroupBox * logging = new QGroupBox(tr("Logging of GA"));

//    QSpinBox *logging_SpinBox = new QSpinBox;
    logging_SpinBox->setPrefix(tr("Save GA state every "));
    logging_SpinBox->setSuffix(tr(" generations"));
    logging_SpinBox->setMinimum(1);
    logging_SpinBox->setMaximum(1000);
    logging_SpinBox->setSingleStep(10);

    QVBoxLayout * logging_layout = new QVBoxLayout;
    logging_layout->addWidget(logging_SpinBox);

    logging->setLayout(logging_layout);

    connect(logging_SpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), config_dialogue, &ConfigDialog::changeSaveFreq);

    /////


    QGroupBox* email_status_group = new QGroupBox(tr("Email addresses to send GA status updates to"));

    QLabel *email_addresses_Label = new QLabel(tr("Email addresses"));
    QPushButton* email_addresses_button = new QPushButton(tr("Add new email address"));
    QGridLayout *email_addresses_Layout = new QGridLayout;
    email_addresses_Layout->addWidget(email_addresses_Label, 0, 0, 1, 2);
    email_addresses_Layout->addWidget(email_address_List, 1, 0 , 1, 2);
    email_addresses_Layout->addWidget(email_addresses_button, 2, 0, 1, 2);

    email_status_group->setLayout(email_addresses_Layout);

    connect(email_address_List, &QListWidget::itemChanged, this, &EAPage::processEmailAddressListChange);
    connect(this, &EAPage::emailAddressListChanged, config_dialogue, &ConfigDialog::changeEmailAddresses);
    connect(email_addresses_button, &QPushButton::clicked, this, &EAPage::addEmailAddress);
//    connect(email_address_List, &QListWidget::itemClicked, this, &EAPage::displaySaveMapHelp);


    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(parameters_Group);
    mainLayout->addWidget(termination_Group);
    mainLayout->addWidget(reseed);
    mainLayout->addWidget(logging);
    mainLayout->addWidget(email_status_group);
    mainLayout->addStretch(1);
    setLayout(mainLayout);
}

void EAPage::updatePop(int pop)
{
    pop_SpinBox->setValue(pop);
}

void EAPage::updateHyprvolTerm(int gen)
{
    hyprvol_term_SpinBox->setValue(gen);
}

void EAPage::updateMaxGenTerm(int gen)
{
    term_SpinBox->setValue(gen);
}

void EAPage::updateReseedFile(QString path)
{
    reseed_edit->setText(path);
}

void EAPage::updateLoggingFreq(int freq)
{
    logging_SpinBox->setValue(freq);
}
void
EAPage::processEmailAddressListChange()
{
    QVector<QString> items;
    for (int j = 0; j < email_address_List->count() ; ++j)
    {
        items.append(email_address_List->item(j)->text());
    }
    emit emailAddressListChanged(items);
}
void
EAPage::updateEmailAddresses(std::vector<std::string> email_addresses)
{
    BOOST_FOREACH(std::string & address, email_addresses)
                {
                    QListWidgetItem * new_item = new QListWidgetItem(email_address_List);
                    new_item->setText(QString::fromStdString(address));
                    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
                }
}
void
EAPage::addEmailAddress()
{
    QListWidgetItem * new_item = new QListWidgetItem(email_address_List);
    new_item->setText(tr("New email address"));
    new_item->setFlags(new_item->flags() | Qt::ItemIsEditable);
}
